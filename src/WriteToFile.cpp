#include "WriteToFile.h"
#include <string.h> 
#include <string> 

using namespace NFileWrapper;
namespace
{
    class CFileWrite: public IFileWrapper
    {
    public:
        void Write(const char *str, std::size_t n) override
        {
            fwrite(str, n, 1, m_fp);
        }
    };

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////


    //структура файла такая: https://ru.wikipedia.org/wiki/Chunked_transfer_encoding
    using pchar =char *;
    class CFileWriteChunked: public IFileWrapper
    {
    public:
        CFileWriteChunked():
          m_bytesLeft(0)
        , m_parseError(false)
        {
        }

    
    void Write(const char *s, std::size_t n) override   
    {          
        for(std::size_t i=0; i < n;)
        {
            if (m_parseError)
            {
                fwrite(s+i, n-i, 1, m_fp);
                return;
            }

            if (m_readingPayload)
                readingPayload(s,n,i);
            else
                readingManagedBytes(s,n,i);
        }  
    }

    private:
        void readingManagedBytes(const char *s, std::size_t n, std::size_t &i)
        {
            switch(m_crlfRead)
            {
                case 0:
                {
                    for(;i<n && m_hexNumber.size() <= m_maxHexSize && s[i]!='\r';++i)
                        m_hexNumber+=s[i];
                    
                    if (i == n)
                        return;

                    if(m_hexNumber.size() > m_maxHexSize)
                    {
                        processParseError();
                        return ;
                    }
                    
                    m_crlfRead = 1;
                    return ;
                }
                case 1:
                {                    
                    if (s[i++] != '\r')
                    {
                        processParseError();
                        return;
                    }

                    m_crlfRead = 2;
                    return ;
                }
                 
                case 2:
                {                    
                    if (s[i++] != '\n')
                    {
                        processParseError();
                        return;
                    }

                    m_crlfRead = 0;
                    m_readingPayload = true;

                    if (!hex_to_ulong(m_hexNumber.c_str(), m_hexNumber.size(), m_bytesLeft))
                        processParseError();
                    else
                        printf("%s\n", m_hexNumber.c_str());                      

                    m_hexNumber.clear();
                    return ;                    
                }
            }
        }

        void readingPayload(const char *s, std::size_t n, std::size_t &i)
        {
            switch(m_crlfRead)
            {
                case 0:
                {
                    std::size_t bytesToFile = std::min(m_bytesLeft, n-i);

                    if(bytesToFile > 0)
                        fwrite(s+i, bytesToFile, 1, m_fp);

                    m_bytesLeft -= bytesToFile;
                    i+=bytesToFile;

                    if (0 == m_bytesLeft)
                        m_crlfRead = 1;

                    return;                    
                }

                case 1:
                {                   
                    if (s[i++] != '\r')
                    {
                        processParseError();
                        return ;
                    }

                    m_crlfRead = 2;
                    return ;
                }

                case 2:
                {                     
                    if (s[i++]!= '\n')
                    {
                        processParseError();                        
                        return;
                    }

                    m_crlfRead = 0;
                    m_readingPayload = false;

                    return;                    
                }

            }

            return;
        }

        void processParseError()
        {
            m_parseError = true;
            printf("Parse Error!!! \n");
        }
        
        static bool hex_to_ulong(const char *sz, std::size_t n, std::size_t &res) 
        {
            if (n<1 )
                return false;

            std::string s;
            s.append(sz, n);

            char * p;
            auto t = std::strtoul(s.c_str(), &p, 16);
            if (*p != 0)
                return false;

            res = t;
            return true;
        }

    private:
        
        std::size_t m_bytesLeft; //payload bytes
        bool m_parseError;

        ////////////////////////////
        bool m_readingPayload = false;
        int m_crlfRead=0;
        std::string m_hexNumber;

        const unsigned m_maxHexSize=8;
    };

}
////////////////////////////////////////////////

namespace NFileWrapper
{
    PFileWrapper CreateFileWrapper(NFileTypes::FileTypes type)
    {
        switch(type)
        {
            case(NFileTypes::Chunked_):
                return std::make_shared<CFileWriteChunked>();
            default: //на строгих настройках компилятора иначе не сделаешь
                return std::make_shared<CFileWrite>();
        }
        
        return std::make_shared<CFileWrite>();
    }
}

