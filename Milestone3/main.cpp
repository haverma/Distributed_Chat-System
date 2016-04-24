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
#include <unistd.h>

struct sockaddr_in sListeningAddr, sMsgListeningAddr;
struct sockaddr_in sRecAddr, sRecMsgAddr;
int iRecAddrLen, iRecMsgAddrLen;
int iListeningSocketFd, iSendingSocketFd;
int iMsgListeningSocketFd, iMsgSendingSocketFd;
int iListeningPortNum = 0, iMsgListeningPortNum = 0;
std::string username;
bool is_server, is_server_alive, declare_leader, leader_already_declared;
bool connection_flag = false, shut_down = true;
std::queue<msg_struct *> qpsBroadcastq;
std::queue<msg_struct *> qpsMsgBroadcastq;
std::list<msg_struct *> lpsClientInfo;
std::list<sockaddr_in *> lpsClients;
std::list<sockaddr_in *> lpsClientsMsg;
std::map<int, msg_struct *> holdbackMap;
std::map<int, msg_struct *> sentBufferMap;
std::map<int, msg_struct *> broadcastBufferMap;
std::list<int> liCurrentClientPort; 
int iSeqNum = 0, iExpSeqNum = 0;
int iMsgId = 0, iResponseCount = 0;
std::mutex seqNumMutex;
std::mutex expSeqNumMutex;
std::mutex msgIdMutex;
std::mutex broadcastMutex;
std::mutex msgBroadcastMutex;
std::mutex clientListMutex;
std::mutex broadcastbufferMutex;
std::mutex sentbufferMutex;
std::mutex heartbeatMutex;
std::mutex newLeaderElectedMutex;
std::mutex displayMutex;
msg_struct sServerInfo, sMyInfo;
sockaddr_in sServerAddr, sServerMsgAddr;

void get_ip_address(char * ip);
void user_listener();
void msg_listener();
void check_ack_sb(int time_diff_sec);
void broadcast_message();
void heartbeat();
void spl_msg_listener();

int main(int argc, char ** argv)
{
    char acBufferLocal[BUFF_SIZE];
    std::string token;
    struct sockaddr_in sConnectingAddr;
    msg_struct * psMsgStruct;
    sockaddr_in * psSockAddr;
    char acTemp[50] = "";
    struct sockaddr_in temp;
    int addrlen = sizeof(temp);

    if(argc != 2 && argc != 3)
    {
        fprintf(stderr,"Incorrect number of arguments supplied. Please retry\n");
        exit(1);
    }

    int i = 0;
    while(argv[1][i] != '\0')
    {
        if(!isalnum(argv[1][i++]))
        {
            fprintf(stdout, "Not a valid name. Please use alpha numeric characters\n");
            exit(1);
        }
    }


    iRecAddrLen = sizeof(sRecAddr);
    iRecMsgAddrLen = sizeof(sRecMsgAddr);

    /* Establishing listener socket */

    sListeningAddr.sin_family = AF_INET;
    sListeningAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sListeningAddr.sin_port = htons(iListeningPortNum);

    iListeningSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iListeningSocketFd < 0)
    {
        fprintf(stderr, "Error while opening listening socket\n");
        exit(1);
    }

    while(!iListeningPortNum)
    {
        if (bind(iListeningSocketFd, (struct sockaddr *) &sListeningAddr, sizeof(sListeningAddr)) < 0)
        {
            fprintf(stderr, "Error while binding listening socket\n");
            exit(1);
        }

        if(getsockname(iListeningSocketFd, (struct sockaddr *) &temp, (socklen_t *) &addrlen) == 0 &&
           temp.sin_family == AF_INET && addrlen == sizeof(temp))
        {
            iListeningPortNum = ntohs(temp.sin_port);
        }
    }

    

    get_ip_address(acTemp);
    if(!strcmp(acTemp, ""))
        strcpy(acTemp, "127.0.0.1");

    /* Establishing msg listener socket */

    sMsgListeningAddr.sin_family = AF_INET;
    sMsgListeningAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sMsgListeningAddr.sin_port = htons(iMsgListeningPortNum);

    iMsgListeningSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iMsgListeningSocketFd < 0)
    {
        fprintf(stderr, "Error while opening msg listening socket\n");
        exit(1);
    }

    while(!iMsgListeningPortNum)
    {
        if (bind(iMsgListeningSocketFd, (struct sockaddr *) &sMsgListeningAddr, sizeof(sMsgListeningAddr)) < 0)
        {
            fprintf(stderr, "Error while binding listening socket\n");
            exit(1);
        }

        if(getsockname(iMsgListeningSocketFd, (struct sockaddr *) &temp, (socklen_t *) &addrlen) == 0 &&
           temp.sin_family == AF_INET && addrlen == sizeof(temp))
        {
            iMsgListeningPortNum = ntohs(temp.sin_port);
        }
    }

    //std::cout << "Message listening port: " << iMsgListeningPortNum << "\n";

    /* Storing my info in sMyInfo struct */
    sMyInfo.name = argv[1];
    sMyInfo.ipAddr = acTemp;
    sMyInfo.port = iListeningPortNum;
    sMyInfo.msgPort = iMsgListeningPortNum;
    sMyInfo.addr = &sListeningAddr;

    /* Establishing sender socket */
    
    iSendingSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iSendingSocketFd < 0)
    {
        fprintf(stderr, "Error while opening sender socket\n");
        exit(1);
    }

    iMsgSendingSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iMsgSendingSocketFd < 0)
    {
        fprintf(stderr, "Error while opening sender socket\n");
        exit(1);
    }

    /* If initiating a new chat */
    if(2 == argc)
    {
        fprintf(stdout, "\n%s started a new chat, listening on %s:%d\n\n", argv[1], acTemp, iListeningPortNum);
        fprintf(stdout, "Succeeded, current users:\n%s %s:%d (Leader)\n\nWaiting for others to join...\n",
                argv[1], acTemp, iListeningPortNum);

        is_server = true;

        /* Set sServerAddr */
        sServerAddr.sin_family = AF_INET;
        if(inet_pton(AF_INET, acTemp, &sServerAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerAddr.sin_port = htons(iListeningPortNum);

        /* Set sServerMsgAddr that contains the addr to which text msgs are to be
         * sent and received */
        sServerMsgAddr.sin_family = AF_INET;
        if(inet_pton(AF_INET, acTemp, &sServerMsgAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerMsgAddr.sin_port = htons(iMsgListeningPortNum);

        /* Adding server info into sServerInfo struct */
        sServerInfo.name = argv[1];
        sServerInfo.ipAddr = acTemp;
        sServerInfo.port = iListeningPortNum;
        sServerInfo.msgPort = iMsgListeningPortNum;

        /* Set username to what's being passed as an arg */
        username = argv[1];

        /* Start all the threads */
        std::thread user_listener_thread(user_listener);
        std::thread msg_listener_thread(msg_listener);
        std::thread spl_msg_listener_thread(spl_msg_listener);
        std::thread broadcast_message_thread(broadcast_message);
        std::thread heartbeat_thread(heartbeat);

        /* Wait for threads to join */
        user_listener_thread.join();
        msg_listener_thread.join();
        spl_msg_listener_thread.join();
        broadcast_message_thread.join();
        heartbeat_thread.join();

    }

    /* If connecting to an already present chat system */
    else
    {
        is_server = false;

        leader_already_declared = true;

        /* Set username to what's being passed as an arg */
        username = argv[1];

        sConnectingAddr.sin_family = AF_INET;

        token = strtok(argv[2]," :");
        if(NULL == token.c_str())
        {
            fprintf(stderr, "The IP address specified is not valid. Please retry\n");
            exit(1);
        }

        if(inet_pton(AF_INET, token.c_str(), &sConnectingAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }

        sServerInfo.ipAddr = token;

        char * pcTempToken;
        pcTempToken = strtok(NULL, " :");
        if(pcTempToken == 0)
        {
            fprintf(stdout, "Incorrect arguments. Please retry\n");
            exit(1);
        }
        token = pcTempToken;

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

        sConnectingAddr.sin_port = htons( (int) strtol(token.c_str(), NULL, 10) );

        sServerInfo.port = atoi(token.c_str());
        
        fprintf(stdout, "\n%s joining a new chat on %s:%d, listening on %s:%d\n\n", argv[1],
                sServerInfo.ipAddr.c_str(), sServerInfo.port, acTemp, iListeningPortNum);

        sprintf(&acBufferLocal[MSG_TYPE], "%d", (int) messageType::REQ_CONNECTION);
        strcpy(&acBufferLocal[NAME], username.c_str());
        sprintf(&acBufferLocal[DATA], "%d", iListeningPortNum);
        int iTempStrLen = DATA + strlen(&acBufferLocal[DATA]) + 1;
        sprintf(&acBufferLocal[iTempStrLen], "%d", iMsgListeningPortNum);

        /* Copying connecting addr info to server addr assuming it is
         * connecting to server. If it is not server, we get to know that when
         * we receive SERVER_INFO */
        sServerAddr = sConnectingAddr;

        /* Start all the threads */
        std::thread user_listener_thread(user_listener);
        std::thread msg_listener_thread(msg_listener);
        std::thread spl_msg_listener_thread(spl_msg_listener);
        std::thread broadcast_message_thread(broadcast_message);
        std::thread heartbeat_thread(heartbeat);
        
        sendto(iSendingSocketFd, acBufferLocal, BUFF_SIZE, 0,
            (struct sockaddr *) &sConnectingAddr, sizeof(sockaddr_in));

        sleep(5);
        if(connection_flag == false)
        {
            fprintf(stdout, "Sorry, no chat is active on %s:%d, try again later.\nBye.\n\n",
                    sServerInfo.ipAddr.c_str(), sServerInfo.port);
            exit(1);
        }
        
        user_listener_thread.join();
        msg_listener_thread.join();
        spl_msg_listener_thread.join();
        broadcast_message_thread.join();
        heartbeat_thread.join();
    }
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
