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
#include <ifaddrs.h>
#include <thread>

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
std::map<int, msg_struct *> holdbackMap;
std::map<int, msg_struct *> sentBufferMap;
std::map<int, msg_struct *> broadcastBufferMap;
int iSeqNum = 0, iExpSeqNum = 0;
int iMsgId = 0;
std::mutex seqNumMutex;
std::mutex msgIdMutex;
std::mutex broadcastMutex;
std::mutex clientListMutex;
std::mutex broadcastbufferMutex;
std::mutex sentbufferMutex;
msg_struct sServerInfo, sMyInfo;
sockaddr_in sServerAddr;

void get_ip_address(char * ip);
void user_listener();
void msg_listener();
void check_ack_sb();
void broadcast_message();


int main(int argc, char ** argv)
{
    std::string token;
    struct sockaddr_in sConnectingProcess;
    msg_struct * psMsgStruct;
    sockaddr_in * psSockAddr;
    char acTemp[50];

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

    //get_ip_address(acTemp);
    //if(NULL == acTemp)
        strcpy(acTemp, "127.0.0.1");

    /* Storing my info in sMyInfo struct */
    sMyInfo.name = username;
    sMyInfo.ipAddr = acTemp;
    sMyInfo.port = iListeningPortNum;
    sMyInfo.addr = &sListeningAddr;

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
        memset(&sServerAddr, 0x0, sizeof(sServerAddr));
        sServerAddr.sin_family = AF_INET;
        if(inet_pton(AF_INET, acTemp, &sServerAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerAddr.sin_port = htons(8216);

        /* Adding the server info in the clients list */
        psMsgStruct = (msg_struct * ) malloc(sizeof(msg_struct));
        if(NULL == psMsgStruct)
        {
            fprintf(stderr, "Error while allocating memory. Please retry\n");
            exit(1);
        }
        psMsgStruct->name = argv[1];
        psMsgStruct->ipAddr = acTemp;
        psMsgStruct->port = 8216;
        clientListMutex.lock();
        lpsClientInfo.push_back(psMsgStruct);
        clientListMutex.unlock();

        /* Adding server addr in the clients list */
        psSockAddr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
        memset(psSockAddr, 0x0, sizeof(psSockAddr));
        psSockAddr->sin_family = AF_INET;
        if(inet_pton(AF_INET, acTemp, &(psSockAddr->sin_addr)) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        psSockAddr->sin_port = htons(8216);
        clientListMutex.lock();
        lpsClients.push_back(psSockAddr);
        clientListMutex.unlock();

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

    std::thread user_listener_thread(user_listener);
    std::thread msg_listener_thread(msg_listener);
    std::thread broadcast_message_thread(broadcast_message);
    std::thread client_chat_ack_thread(check_ack_sb);

    user_listener_thread.join();
    msg_listener_thread.join();
    broadcast_message_thread.join();
    client_chat_ack_thread.join();

    return 0;
}

void get_ip_address(char * ip)
{
    struct ifaddrs * sIfAddr = NULL;
    struct ifaddrs * sIterator = NULL;
    void * pvTemp = NULL;

    strcpy(ip, "");
    getifaddrs(&sIfAddr);

    for(sIterator = sIfAddr; sIterator != NULL; sIterator = sIterator->ifa_next)
    {
        if(!sIterator->ifa_addr)
        {
            continue;
        }

        if( (sIterator->ifa_addr->sa_family == AF_INET) &&
                !(strcmp(sIterator->ifa_name, "em1")))
        {
            pvTemp = &((struct sockaddr_in *) sIterator->ifa_addr)->sin_addr;
            char acAddrBuff[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, pvTemp, acAddrBuff, INET_ADDRSTRLEN);
            strcpy(ip, acAddrBuff);
        }
    }
    if(sIfAddr != NULL)
        freeifaddrs(sIfAddr);

    return;
}
