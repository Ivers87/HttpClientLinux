#include "WriteToFile.h"
#include <string.h> 
#include <string> 

using namespace NFileWrapper;
namespace
{
    class CFileWrite: public IFileWrapper
    {
    public:
        void Write(char *str, std::size_t n) override
        {
            fwrite(str, n, 1, m_fp);
        }
    };

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////


    //структура файла такая: https://ru.wikipedia.org/wiki/Chunked_transfer_encoding
    // в начало добавляю CRLF, после этого имею такую структуру: 
    //[CRLF<HEX>CRLF<Payload>]+CRLF  , произвожу парсинг
    //0 не нужно особо обрабатывать: это chunk с 0 байт

    using pchar =char *;
    class CFileWriteChunked: public IFileWrapper
    {
    public:
        CFileWriteChunked():
          m_bytes_left(0)
        , m_prefix("\r\n")
        , m_parseError(false)
        {
        }

    
    void Write(char *s, std::size_t n) override   
    {  
        if (m_parseError)
        {
            fwrite(s, n, 1, m_fp);
            return;
        }

        const char *sEnd=s+n;

        if (!m_prefix.empty())
        {
            s-=m_prefix.size();
            strncpy(s,m_prefix.c_str(),m_prefix.size());
            m_prefix.clear();
        }        

        for(;s != sEnd;)
        {
            if (0 == m_bytes_left)
            {
                int status = parseBegin(s,sEnd-s,m_bytes_left);

                if (0 == status)
                {
                    m_prefix=s;
                    return;
                }

                if (-1 == status)
                {
                    m_parseError = true;//всё оставшееся пишем в файл, даже с управляющими байтами
                    printf("Error in parsing chunked\n");
                    fwrite(s, n, 1, m_fp);

                    return;
                }
            }
            else
            {
                std::size_t nn = std::min(m_bytes_left, (std::size_t)(sEnd-s));
                fwrite(s, nn, 1, m_fp);

                m_bytes_left -= nn;
                s+=nn;
            }    
        }   
    }

    private:
        
        //0 - если паттерн [<HEX><CR>] | [<HEX>]  
        // иначе -1
        static int parseBeginPart(const char *s, std::size_t n) 
        {
            if (0 == n)
                return 0;

            if ('\r' == s[n-1])
                --n;

            std::size_t res;            
            return  hex_to_ulong(s,n,res) ? 0 : -1;
        }

        //<CR><LF><HEX-number><CR><LF>
        //1 - если мы стоим на начале паттерна. Сдвигаем s на позицию после паттерна, заполняем m_bytes_left     
        //0 - если мы на "середине" этого паттерна, <CR><LF> например,  m_bytes_left  не меняется
        //-1 - ошибка, мы не стоим в начале этого паттерна
        static int parseBegin(pchar &s, std::size_t n, std::size_t &bytes_left)
        {        
            if (1 == n)
                return (*s)=='\r'?0:-1;

            // далее их >=2
            if ((*s)!='\r' || *(s+1)!='\n')
                return -1;

            char *s2=strstr(s+1, "\r\n");

            if (NULL == s2)
                return parseBeginPart(s+2, n-2);    

            fwrite(s+2,s2-s,1,stdout);     

            if (! hex_to_ulong(s+2,s2-s-2, bytes_left))
                return -1;

            s=s2+2;
            return 1;
        }

        static bool hex_to_ulong(const char *sz, std::size_t n, std::size_t &res) 
        {
            if (n<1 || n > RESERVED-BLOCKDELIM)
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
        
        std::size_t m_bytes_left; //payload bytes
        std::string m_prefix;
        bool m_parseError;
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

