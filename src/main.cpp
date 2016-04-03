#include <stdio.h>
#include <iostream>
#include <map>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

struct sockaddr_in sServerListeningAddr, sServerSendingAddr;
struct sockaddr_in sClientListeningAddr, sClientSendingAddr;
int iClientAddrLen = 0;
int iListeningSocketFd, iSendingSocketFd;
int iListeningPortNum = 8216, iSendingPortNum = 8217;
std::string username;
bool is_server;

int main(int argc, char ** argv)
{
    std::string token;
    struct sockaddr_in sConnectingProcess;

    if(argc != 2 && argc != 3)
    {
        fprintf(stderr,"Incorrect number of arguments supplied. Please retry\n");
        exit(1);
    }

    printf("\nThis server listens to port number '8216'\n");

    /* If initiating a new chat */
    if(2 == argc)
    {
        /* Establishing listener socket */
        
        is_server = true;

        memset(&sServerListeningAddr, 0x0, sizeof(sServerListeningAddr));
        sServerListeningAddr.sin_family = AF_INET;
        sServerListeningAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sServerListeningAddr.sin_port = htons(iListeningPortNum);

        iListeningSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

        if (iListeningSocketFd < 0)
        {
            fprintf(stderr, "Error while opening listening socket\n");
            exit(1);
        }

        if (bind(iListeningSocketFd, (struct sockaddr *) &sServerListeningAddr, sizeof(sServerListeningAddr)) < 0)
        {
            fprintf(stderr, "Error while binding listening socket\n");
            exit(1);
        }

        /* Establishing sender socket */
        
        memset(&sServerSendingAddr, 0x0, sizeof(sServerSendingAddr));
        sServerSendingAddr.sin_family = AF_INET;
        sServerSendingAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sServerSendingAddr.sin_port = htons(iSendingPortNum);

        iSendingSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

        if (iSendingSocketFd < 0)
        {
            fprintf(stderr, "Error while opening sender socket\n");
            exit(1);
        }

        if (bind(iSendingSocketFd, (struct sockaddr *) &sServerSendingAddr, sizeof(sServerSendingAddr)) < 0)
        {
            fprintf(stderr, "Error while binding sender socket\n");
            exit(1);
        }

        /* Set username to what's being passed as an arg */
        username = argv[1];
    }

    /* If connecting to an already present chat system */
    else
    {
        /* Establishing listener socket */
        
        is_server = false;

        memset(&sClientListeningAddr, 0x0, sizeof(sClientListeningAddr));
        sClientListeningAddr.sin_family = AF_INET;
        sClientListeningAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sClientListeningAddr.sin_port = htons(iListeningPortNum);

        iListeningSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

        if (iListeningSocketFd < 0)
        {
            fprintf(stderr, "Error while opening listening socket\n");
            exit(1);
        }

        if (bind(iListeningSocketFd, (struct sockaddr *) &sClientListeningAddr, sizeof(sClientListeningAddr)) < 0)
        {
            fprintf(stderr, "Error while binding listening socket\n");
            exit(1);
        }

        /* Establishing sender socket */
        
        memset(&sClientSendingAddr, 0x0, sizeof(sClientSendingAddr));
        sClientSendingAddr.sin_family = AF_INET;
        sClientSendingAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sClientSendingAddr.sin_port = htons(iSendingPortNum);

        iSendingSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

        if (iSendingSocketFd < 0)
        {
            fprintf(stderr, "Error while opening sender socket\n");
            exit(1);
        }

        if (bind(iSendingSocketFd, (struct sockaddr *) &sClientSendingAddr, sizeof(sClientSendingAddr)) < 0)
        {
            fprintf(stderr, "Error while binding sender socket\n");
            exit(1);
        }

        /* Set username to what's being passed as an arg */
        username = argv[1];

        memset(&sConnectingProcess, 0x0, sizeof(sConnectingProcess));
        sConnectingProcess.sin_family = AF_INET;

        token = strtok(argv[2]," :");
        if(NULL == token.c_str())
        {
            fprintf(stderr, "The IP address specified is not valid. Please retry\n");
            exit(1);
        }

        if(inet_pton(AF_INET, token.c_str(), &sConnectingProcess.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }

        token = strtok(NULL, " :");
        if(NULL == token.c_str())
        {
            fprintf(stderr, "The port specified is not valid. Please retry\n");
            exit(1);
        }

        if(-1 == (int) strtol(token.c_str(), NULL, 10))
        {
            fprintf(stderr, "The port specified is not valid. Please retry\n");
            exit(1);
        }

        sConnectingProcess.sin_port = htons( (int) strtol(token.c_str(), NULL, 10) );
    }

    /* Working on the below part
    std::thread user_listener_thread(user_listener);

    std::thread msg_listener_thread(msg_listener);
    */
}
