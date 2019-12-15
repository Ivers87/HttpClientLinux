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
        CHeaders():
        m_transfer_encoding(Default_)
        , m_gotstatus(false)
        {}


        //требования: при первом вызове n достаточно большое, чтоб туда влезла 1я строка заголовков: HTTP/1.1 302 Found
        void Add(const char *s, std::size_t n);
        bool BadStatus(std ::string &err);
        void Release()
        {
            analyze();
            realise();
        }
        bool StatusGot() const
        {
            return m_gotstatus;
        } 

        NFileTypes::FileTypes GetType() const;
    private:
        void analyze();
        void realise()
        {
            m_ss.clear();
        }

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

    private:
        std::string m_s;
        std::stringstream m_ss;

        ETransferEncoding m_transfer_encoding ;
        bool m_gotstatus;
    };
}

#endif