#include <stdio.h> 

#include <arpa/inet.h>
#include <unistd.h> 
#include <string.h> 

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
        CSockHandler hndlr(sock); //обычно через BOOST_SCOPE_EXIT такие вещи делаю
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

        
        const int BUFFSIZE=41;

        char buffer[BUFFSIZE]; 
        bool headers_recvd = false;

        int iResult;

        NFileWrapper::PFileWrapper pFile;
        NHeadersAnalyzer::CHeaders   headers;        

        do {

            iResult = recv(sock, buffer, BUFFSIZE, 0);        
            if (iResult > 0)
            {
                if (headers_recvd)
                {
                    pFile->Write(buffer, iResult);
                }
                else
                {
                    headers.Add(buffer, iResult);

                    if (!headers.StatusGot() && headers.BadStatus(err))
                        return false;
                        
                    std::string bodyBytes;
                    if (headers.HeadersRecieved(bodyBytes))
                    {
                        headers_recvd = true;

                        headers.Analyze();

                        pFile = NFileWrapper::CreateFileWrapper(headers.GetType());

                        if (!pFile->Init(m_filename))
                        {
                            err="can't create file";
                            return false;
                        }

                        pFile->Write(bodyBytes.c_str(), bodyBytes.size());
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

    //link = "http://komissionka31.ru/file/8572_%D0%9E%D0%B1%D1%89%D0%B8%D0%B5/184184_%D0%9D%D0%BE%D0%B2%D1%8B%D0%B9_%D1%82%D0%B5%D0%BA%D1%81%D1%82%D0%BE%D0%B2%D1%8B%D0%B9_%D0%B4%D0%BE%D0%BA%D1%83%D0%BC%D0%B5%D0%BD%D1%82.txt";
    //filename = "komiss.txt";

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

