#include <stdio.h> 
#include <sys/socket.h> 

#include <netdb.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h> 
#include <string.h> 

#include <string> 

#include "WriteToFile.h"
#include "HeadersAnalyzer.h"
#include "UrlParser.h"
#include "Utility.h"

class CSockHandler
{
public:
  CSockHandler(int &sock) :
    m_sock(sock)
  {}

  ~CSockHandler()
  {
    if (m_sock>0)
        close(m_sock);
  }
private:
  int &m_sock;
};

class CCurl
{
public:
    CCurl(const std::string &ip, const std::string &path, const std::string &hostname, int port, const std::string &filename):
      m_ip(ip)
    , m_path(path)
    , m_hostname(hostname)
    , m_filename(filename)
    , m_port(port)
    {     
    }

    //true - OK
    bool Start(std::string &err)
    {
        using namespace NFileWrapper;

        int sock = 0; 
        CSockHandler hndlr(sock);
        struct sockaddr_in serv_addr; 
        
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
            err = "can't init socket"; 
            return false; 
        } 
    
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(m_port); 
        
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr)<=0)  
        { 
            err = "can't resolve ip"; 
            return false; 
        } 
    
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            err = "can't connect"; 
            return false; 
        } 

        send(sock , createRequest().c_str(), createRequest().size() , 0 ); 
        printf("Message sent\n"); 

        
        const int BUFFSIZE=1024;
        const int NN=4; //size of CRLFCRLF
        const std::string ptrn="\r\n\r\n";
        char buffer[BUFFSIZE+NN+RESERVED] = {0}; 
        int shft=0;
        bool headers_recvd = false;

        int iResult;

        NFileWrapper::PFileWrapper pFile;
        NHeadersAnalyzer::CHeaders   headers;        

        do {

            //структура : <Headers>CRLFCRLF<Body>
            //идея: если за 1 раз не дочитали до CRLFCRLF - то, чтоб не пропустить эту последовательность, последние 3 байта предыдущей строки сохраняем вначале 
            //(buffer+RESERVED), новую порцию считываем в (buffer+RESERVED+shft)
            //в boost.asio делается проще: async_read_until(..,  "\r\n\r\n",   ...); 

            char *begin = buffer+RESERVED+shft;
            iResult = recv(sock, begin, BUFFSIZE, 0);        
            if (iResult > 0)
            {
                begin[iResult]='\0';
                if (headers_recvd)
                {
                    pFile->Write(begin, iResult);
                }
                else
                {
                    headers.Add(begin, iResult);

                    if (!headers.StatusGot() && headers.BadStatus(err))
                        return false;

                    char *pos=strstr(begin-shft, ptrn.c_str());

                    if (pos)
                    {
                        headers.Release();                        
                        pFile = NFileWrapper::CreateFileWrapper(headers.GetType());

                        if (!pFile->Init(m_filename))
                        {
                            err="can't create file";
                            return false;
                        }
                        pFile->Write(pos+NN, (begin-pos)+iResult-NN);

                        headers_recvd=true;
                        shft=0;
                    }
                    else //не считали весь заголовок
                    {
                        strncpy(buffer+RESERVED,begin+iResult-(NN-1),NN-1);//последние 3 байта в начало, чтоб не пропустить \r\n\r\n
                        shft=NN-1;
                    }              
                }                    
            }
            else if (0 == iResult)
                printf("Connection closed\n");
            else
            {
                err="recv error";
                return false;
            }

        } while (iResult > 0);

        return true;
    }
private:
    std::string createRequest()const
    {
        std::string req;

        req = "GET " + m_path + std::string( " HTTP/1.1\r\n");
        req+= "Host: " + m_hostname + std::string("\r\n");
        req+= "User-Agent: curl\r\n";
        req+= "Accept: */*\r\n";
        req+= "Connection: close\r\n\r\n";
        req+= "\r\n";

        return req;
    } 
private:
    const std::string m_ip;
    const std::string m_path;
    const std::string m_hostname;
    const std::string m_filename;

    int m_port;
};  

//////////////////////////////////////////////////////

int main(int argc, char const *argv[]) 
{     
    printf("Begin curl_ivb\n"); 

    if (argc < 2)
    {
        printf("USAGE: curl_ivb <link> [filename]\n");
        return 1;
    }

    std::string link = argv[1];
    std::string filename;

    if (argc >=3 )
        filename = argv[2];

    printf("%s %s\n", link.c_str(), filename.c_str()); 

    CUrlParser u(link, filename.empty());
    printf("host= %s| path= %s|\n", u.GetHost().c_str(), u.GetPath().c_str());

    std::string ip;
    if (!NUtility::GetIP(u.GetHost(), ip))
        return 1;

    printf("ip = %s\n", ip.c_str()); 

    CCurl C(ip.c_str(), u.GetPath(), u.GetHost(), u.GetPort(),
            filename.empty()?u.GetFilename():filename);
    
    try
    {
        std::string err;
        if (!C.Start(err))
        {
            printf("Error: %s\n", err.c_str()); 
            return 1;
        }
    }
    catch(std::bad_alloc &)
    {
        printf("Error: bad_alloc\n"); 
        return 1;
    }

    printf("End curl_ivb OK\n"); 

    return 0; 
} 

