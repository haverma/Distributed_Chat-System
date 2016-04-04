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
#include "./chat_system.h"

struct sockaddr_in sListeningAddr;
struct sockaddr_in sRecAddr;
int iRecAddrLen;
int iListeningSocketFd, iSendingSocketFd;
int iListeningPortNum = 8216;
std::string username;
bool is_server;
std::queue<msg_struct *> qpsBroadcastq;
std::list<msg_struct *> lpsClientInfo;
std::list<sockaddr_in *> lpsClients;
int iSeqNum;
int iMsgId;
std::mutex seqNumMutex;
std::mutex msgIdMutex;
std::mutex broadcastMutex;
std::mutex clientListMutex;

int main(int argc, char ** argv)
{
    std::string token;
    struct sockaddr_in sConnectingProcess;

    if(argc != 2 && argc != 3)
    {
        fprintf(stderr,"Incorrect number of arguments supplied. Please retry\n");
        exit(1);
    }

    printf("\nThis user listens to port number '8216'\n");

    iRecAddrLen = sizeof(sRecAddr);

    /* Establishing listener socket */

    memset(&sListeningAddr, 0x0, sizeof(sListeningAddr));
    sListeningAddr.sin_family = AF_INET;
    sListeningAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sListeningAddr.sin_port = htons(iListeningPortNum);

    iListeningSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iListeningSocketFd < 0)
    {
        fprintf(stderr, "Error while opening listening socket\n");
        exit(1);
    }

    if (bind(iListeningSocketFd, (struct sockaddr *) &sListeningAddr, sizeof(sListeningAddr)) < 0)
    {
        fprintf(stderr, "Error while binding listening socket\n");
        exit(1);
    }

    /* Establishing sender socket */
    
    iSendingSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iSendingSocketFd < 0)
    {
        fprintf(stderr, "Error while opening sender socket\n");
        exit(1);
    }

    /* If initiating a new chat */
    if(2 == argc)
    {
        is_server = true;
        
        /* Set username to what's being passed as an arg */
        username = argv[1];
    }

    /* If connecting to an already present chat system */
    else
    {
        is_server = false;

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
