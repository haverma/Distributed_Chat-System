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
#include <signal.h>
#include <sys/syscall.h>

#include "mainwindow.h"
#include <QApplication>

struct sockaddr_in sListeningAddr;
struct sockaddr_in sRecAddr;
int iRecAddrLen;
int iListeningSocketFd, iSendingSocketFd;
int iListeningPortNum = 0;
std::string username;
bool is_server, is_server_alive, declare_leader, leader_already_declared;
std::queue<msg_struct *> qpsBroadcastq;
std::list<msg_struct *> lpsClientInfo;
std::list<sockaddr_in *> lpsClients;
std::map<int, msg_struct *> holdbackMap;
std::map<int, msg_struct *> sentBufferMap;
std::map<int, msg_struct *> broadcastBufferMap;
std::list<int> liCurrentClientPort; 
int iSeqNum = 0, iExpSeqNum = 0;
int iMsgId = 0, iResponseCount = 0;
std::mutex seqNumMutex;
std::mutex msgIdMutex;
std::mutex broadcastMutex;
std::mutex clientListMutex;
std::mutex broadcastbufferMutex;
std::mutex sentbufferMutex;
std::mutex heartbeatMutex;
std::mutex newLeaderElectedMutex;
msg_struct sServerInfo, sMyInfo;
sockaddr_in sServerAddr;

//#include "globals.h"
MainWindow* w;

void get_ip_address(char * ip);
void user_listener();
void msg_listener();
void check_ack_sb(int time_diff_sec);
void broadcast_message();
void heartbeat();


void interfaceLoop(int argc, char* argv[])
{
    //pid_t parent = getpid();
    //printf("PID = %d\n", parent );
    QApplication a(argc, argv);
    w = new MainWindow(NULL);

    // window title
    w->setWindowTitle(username.c_str());
    // stylesheet
    w->setStyleSheet("QWidget { background-color: #222222 }"
                     "QMainWindow { background-color: #222222 }"
                     "QPlainTextEdit { background-color: #222200 }"
                     "QPlainTextEdit { color: #88ffff }"
                     "QListWidget { background-color: #302828 }"
                     "QListWidget { color: #ff5555 }"
                     "QTextBrowser { background-color: #101515 }"
                     "QTextBrowser { color: #22ffff }"
                     "QLabel { color: #ffffff }");

    w->updateServerLabel(is_server);
/*
    "background-color: #113333;"
    "selection-color: #113333;"
    "selection-background-color: blue;"
*/
    w->show();

    a.exec();
    delete w;

    // send closing signal
    char acBuffer[BUFF_SIZE] = "";
    int iTemp = 0;
    msg_struct * psMsg = NULL;
    int iSocketFd;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iSocketFd < 0)
    {
        fprintf(stderr, "Error while opening socket\n");
        exit(1);
    }

    if(!is_server)
    {
        // Store CHAT msg into acBuffer and send it to the server
        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::CLIENT_EXITED);
        sprintf(&acBuffer[DATA], "%d", iListeningPortNum);
        sendto(iSocketFd, acBuffer, BUFF_SIZE, 0,
        (struct sockaddr *) &sServerAddr, sizeof(sockaddr_in));
        exit(1);
    }
    else
    {
        std::cout<<"Exiting the chat application... Server Closing.. !!"<<"\n";
        exit(1);
    }
    // done sending close signal

    //kill(parent, SIGEV_THREAD_ID);
    std::terminate();
    printf("Closed application\n");
}

void interface(void)
{
    interfaceLoop(0, NULL);
}

int main(int argc, char ** argv)
{
    char acBufferLocal[BUFF_SIZE];
    std::string token;
    struct sockaddr_in sConnectingAddr;
    msg_struct * psMsgStruct;
    sockaddr_in * psSockAddr;
    char acTemp[50];
    struct sockaddr_in temp;
    int addrlen = sizeof(temp);

    if(argc != 2 && argc != 3)
    {
        fprintf(stderr,"Incorrect number of arguments supplied. Please retry\n");
        exit(1);
    }


    iRecAddrLen = sizeof(sRecAddr);

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

    printf("\nThis user listens to port number '%d'\n", iListeningPortNum);

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
        is_server = true;

        /* Set sServerAddr */
        sServerAddr.sin_family = AF_INET;
        if(inet_pton(AF_INET, acTemp, &sServerAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerAddr.sin_port = htons(iListeningPortNum);

        /* Adding server info into sServerInfo struct */
        sServerInfo.name = argv[1];
        sServerInfo.ipAddr = acTemp;
        sServerInfo.port = iListeningPortNum;

        /* Set username to what's being passed as an arg */
        username = argv[1];

        /* Start all the threads */
        std::thread interface_thread(interface); while(!w){usleep(100);}
        //std::thread user_listener_thread(user_listener);
        std::thread msg_listener_thread(msg_listener);
        std::thread broadcast_message_thread(broadcast_message);
        std::thread heartbeat_thread(heartbeat);

        /* Wait for threads to join */
        interface_thread.join();
        //user_listener_thread.join();
        msg_listener_thread.join();
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

        sConnectingAddr.sin_port = htons( (int) strtol(token.c_str(), NULL, 10) );

        sServerInfo.port = atoi(token.c_str());

        sprintf(&acBufferLocal[MSG_TYPE], "%d", (int) messageType::REQ_CONNECTION);
        strcpy(&acBufferLocal[NAME], username.c_str());
        sprintf(&acBufferLocal[DATA], "%d", iListeningPortNum);
        sendto(iSendingSocketFd, acBufferLocal, BUFF_SIZE, 0,
            (struct sockaddr *) &sConnectingAddr, sizeof(sockaddr_in));

        /* Copying connecting addr info to server addr assuming it is
         * connecting to server. If it is not server, we get to know that when
         * we receive SERVER_INFO */
        sServerAddr = sConnectingAddr;

        /* Start all the threads */
        std::thread interface_thread(interface);  while(!w){usleep(100);}
        //std::thread user_listener_thread(user_listener);
        std::thread msg_listener_thread(msg_listener);
        std::thread broadcast_message_thread(broadcast_message);
        std::thread heartbeat_thread(heartbeat);

        interface_thread.join();
        //user_listener_thread.join();
        msg_listener_thread.join();
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

