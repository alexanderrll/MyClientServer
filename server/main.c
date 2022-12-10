#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3000" //port for users
#define BACKLOG 10  //len count wait connect

void sigchld_handler()
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//get sockaddr ipv4
struct in_addr *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return NULL;
}

int main()
{
    int sockfd = 0, //listening
        new_fd = 0, //new connection
        status = 0; //status

    struct addrinfo hints,
                    *p_servinfo = NULL,
                    *p = NULL;

    struct sockaddr_storage their_addr; //address info about client
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          //ipv4
    hints.ai_socktype = SOCK_STREAM;    //tcp
    hints.ai_flags = AI_PASSIVE;
    if((rv = getaddrinfo(NULL, PORT, &hints, &p_servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo errpr: %s\n", gai_strerror(status));
        exit(0);
    }

    for(p = p_servinfo; p != NULL; p = p->ai_next)
    {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            fprintf(stderr, "server error:socket\n");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            fprintf(stderr, "server error: setcockopt\n");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            fprintf(stderr, "server error:bind\n");
            continue;
        }
        break;
    }

    if(p == NULL)
    {
        fprintf(stderr, "server error:bind\n");
        exit(2);
    }

    freeaddrinfo(p_servinfo);

    if(listen(sockfd, BACKLOG) == -1)
    {
        fprintf(stderr, "server error:listen\n");
        exit(3);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        fprintf(stderr, "server error:SIGACTION\n");
        exit(4);
    }

    fprintf(stderr, "server:waiting for any connections...\n");

    while(1)
    {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if(new_fd == -1)
        {
            fprintf(stderr, "server error:accept\n");
            continue;
        }
        
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        fprintf(stderr, "server:got connection from %s\n", s);

        if(!fork())
        {
            close(sockfd);

            while(1) {
                char input_buff[256];
                memset(&input_buff, 0, sizeof(input_buff));
                scanf("%s", &input_buff);
                send(new_fd, &input_buff, 13, 0);
                fprintf(stderr, "server:send\n");
            }
                exit(5);

            close(new_fd);
                exit(0);
        }
        close(new_fd);
    }
}
