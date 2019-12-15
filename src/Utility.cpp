#include <stdio.h> 

#include <netdb.h>
#include <arpa/inet.h>

#include <string.h> 

#include "Utility.h"

namespace NUtility
{
    bool GetIP (const std::string &addp, std::string &sres)
    {        
        struct addrinfo hints, *res, *p;
        int status;
        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(addp.c_str(), NULL, &hints, &res)) != 0)
        {
            printf("Error: can't get ip\n");
            return false;
        }

        for(p = res;p != NULL; p = p->ai_next)
        {
            void *addr;

            // Get the pointer to the address itself, different fields in IPv4 and IPv6

            if (p->ai_family == AF_INET)
            {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
            }
            else
            {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
            }

            char res_ip[INET_ADDRSTRLEN];
            inet_ntop(p->ai_family, addr, res_ip, INET_ADDRSTRLEN);

            sres=res_ip;

            return true;
        }

        return false;
    }

}