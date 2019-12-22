#ifndef HEADERS_ANALYZER__
#define HEADERS_ANALYZER__

#include <string>
#include <sstream> 
#include <algorithm>
#include "FileTypes.h"

namespace NHeadersAnalyzer
{
    class CHeaders
    {
        enum ETransferEncoding {Default_=0, Chunked_};
    public:
        CHeaders()
        {
            clear();
        }


        //требования: при первом вызове n достаточно большое, чтоб туда влезла 1я строка заголовков: HTTP/1.1 302 Found
        void Add(const char *s, std::size_t n);
        bool BadStatus(std ::string &err);
        void Analyze();
        void Clear()
        {
            clear();
        }

        bool StatusGot() const
        {
            return m_gotstatus;
        }      

        NFileTypes::FileTypes GetType() const
        {
            if (Chunked_ == m_transfer_encoding)
                return NFileTypes::Chunked_;

            return NFileTypes::Common_;
        }        

        bool HeadersRecieved(std::string &);
    private:
        static void split(const std::string &header, std::string &name, std::string &value)
        {
            std::stringstream s(header);
            s >> name >> value;
        }

        static std::string to_upper_copy(const std::string &s)
        {
            std::string res;
            res.reserve(s.size());

            for_each(s.begin(), s.end(), [&](char c) {res.push_back(std::toupper(c)); });

            return res;
        }

        void analyze_transfer_header(const std::string &value)
        {
            if ("chunked" == value)
            {
                m_transfer_encoding = Chunked_;
                return;
            }
        }

        void clear()
        {
            m_transfer_encoding = Default_;
            m_gotstatus = false;
            m_s.clear();
            m_searchPos = 0;
        }

    private:
        std::string m_s;        
        std::size_t m_searchPos;

        ETransferEncoding m_transfer_encoding ;
        bool m_gotstatus;        
    };
}

#endif