#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT "3000"

#define MAXDATASIZE 100

void printAddrInfo(struct addrinfo *addrinfo)
{
    fprintf(stdout, "addrinfo->ai_flags = %d\n", addrinfo->ai_flags);
    fprintf(stdout, "addrinfo->ai_family = %d\n", addrinfo->ai_family);
    fprintf(stdout, "addrinfo->ai_socktype = %d\n", addrinfo->ai_socktype);
    fprintf(stdout, "addrinfo->ai_protocol = %d\n", addrinfo->ai_protocol);
    //fprintf(stdout, "IPv4= %d\n", addrinfo->ai_protocol);
}

struct in_addr *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return NULL;
}

int main()
{
    int status = 0,
        sockfd = 0,     //descr socket
        numbytes = 0;
    char *ipv4_connect = "127.0.0.1",
         s[INET6_ADDRSTRLEN],
         buf[MAXDATASIZE];

    struct addrinfo hints, 
                    *p_servinfo = NULL, //results
                    *p = NULL;          //

    memset(&hints, 0, sizeof(hints));   //clear struct
    hints.ai_family = AF_INET;          //ipv4
    hints.ai_socktype = SOCK_STREAM;    //tcp
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(ipv4_connect, PORT, &hints, &p_servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo errpr: %s\n", gai_strerror(status));
        return 1;
    }


    for(p = p_servinfo; p!= NULL; p = p->ai_next)
    {

        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            fprintf(stderr, "client:socket\n");
            continue;
        }

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == 01)
        {
            close(sockfd);
            fprintf(stderr, "client:connect\n");
            continue;
        }
        break;
    }

    freeaddrinfo(p_servinfo);

    if(p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    fprintf(stdout, "client: connecting to %s\n", s);

    while(1) {
        recv(sockfd, buf, MAXDATASIZE - 1, 0);
    }

   // if((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
   // {
        //fprintf(stdout, "client: connecting to %s\n", s);
     //   return 3;
   // }

    buf[numbytes] = '\0';

    fprintf(stdout, "client: recieved '%s'\n", buf);
    close(sockfd);
}
