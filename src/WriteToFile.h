#ifndef WRITE_TO_FILE_
#define WRITE_TO_FILE_

#include <stdio.h> 
#include <sys/socket.h> 

#include <netdb.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h> 
#include <string.h> 

#include <string> 
#include <memory> 

#include "FileTypes.h"

namespace NFileWrapper
{
    const int BLOCKDELIM=4;
    const int RESERVED=16+BLOCKDELIM; //максимум: 16-значное число в hex в начале chunk-а
    //thread-unsafe!!!
    class IFileWrapper
    {
    public:
        IFileWrapper()
            :m_fp(NULL)
        {
        }


        //требования:
        // - до &str[0] должно быть зарезервированно RESERVED байт
        // - str[n] == '\0'
        virtual void Write(char *str, std::size_t n) = 0;


        bool Init(const std::string &file)
        {
            m_fp = fopen(file.c_str(), "wb");
            return m_fp != NULL;
        }   

        void Destroy()
        {
            if (m_fp)
            {
                fclose(m_fp);
                m_fp=NULL;
            }
        }
        virtual ~IFileWrapper()
        {
            Destroy();
        }
    protected:
        FILE *m_fp;
    };

    using PFileWrapper = std::shared_ptr<IFileWrapper>;

    PFileWrapper CreateFileWrapper(NFileTypes::FileTypes type);
}

#endif