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
#include "../test_leader_election/chat_system.h"
#include <ifaddrs.h>
#include <thread>

#include "mainwindow.h"
#include <QApplication>

struct sockaddr_in sListeningAddr;
struct sockaddr_in sRecAddr;
int iRecAddrLen;
int iListeningSocketFd, iSendingSocketFd;
int iListeningPortNum; // = 8218;
std::string username;
bool is_server;
std::queue<msg_struct *> qpsBroadcastq;
std::list<msg_struct *> lpsClientInfo;
std::list<sockaddr_in *> lpsClients;
std::map<int, msg_struct *> holdbackMap;
std::map<int, msg_struct *> sentBufferMap;
std::map<int, msg_struct *> broadcastBufferMap;
std::list<int> liCurrentClientPort;
int iSeqNum = 0, iExpSeqNum = 0;
int iMsgId = 0;
std::mutex seqNumMutex;
std::mutex msgIdMutex;
std::mutex broadcastMutex;
std::mutex clientListMutex;
std::mutex broadcastbufferMutex;
std::mutex sentbufferMutex;
std::mutex CurrentClientsListMutex;
msg_struct sServerInfo, sMyInfo;
sockaddr_in sServerAddr;

void get_ip_address(char * ip);
void user_listener();
void msg_listener();
void check_ack_sb();
void broadcast_message();
void client_heartbeat();


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
