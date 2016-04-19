#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include <string>
#include <list>
#include "chat_system.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>

void initiate_leader_election()
{
    struct sockaddr_in * psAddr;
    int iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    int iAddrLen = sizeof(sockaddr_in);
    char acBuffer[BUFF_SIZE] = "";
    bool bDeclareLeader;
    msg_struct * psMsgStruct;

    sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::REQ_LEADER_ELECTION);
    sprintf(&acBuffer[SENDER_LISTENING_PORT], "%d", iListeningPortNum);

    clientListMutex.lock();

    heartbeatMutex.lock();
    declare_leader = true;
    heartbeatMutex.unlock();

    for(std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
    {
        /* Ignore if the entry belongs to the current process */
        if((*i)->name == username && (*i)->port == iListeningPortNum)
            continue;

        /* Create sockaddr struct and add it to lpsClients */
        psAddr = new sockaddr_in;
        psAddr->sin_family = AF_INET;
        if(inet_pton(AF_INET, ((*i)->ipAddr).c_str(), &(psAddr->sin_addr)) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        psAddr->sin_port = htons((*i)->port);

        lpsClients.push_back(psAddr);
        
        if((*i)->port > iListeningPortNum)
        {
            std::cout << "Sending REQ_LEADER_ELECTION\n";

            sendto(iSockFd, acBuffer, BUFF_SIZE * sizeof(char), 0,
                    (struct sockaddr *) psAddr, iAddrLen);
        }
    }
    clientListMutex.unlock();

    sleep(3);

    heartbeatMutex.lock();
    bDeclareLeader = declare_leader;
    heartbeatMutex.unlock();

    newLeaderElectedMutex.lock();
    //std::cout << "leader_already_declared value = " << leader_already_declared << "\n";    
    if(bDeclareLeader && !leader_already_declared)
    {
        /* Remove its entry from the client info list */
        clientListMutex.lock();
        for (std::list<msg_struct *>::iterator iterate = lpsClientInfo.begin(); iterate != lpsClientInfo.end(); ++iterate)
        {
            if((*iterate)->name == username && (*iterate)->port == iListeningPortNum)
            {
                delete *iterate;
                lpsClientInfo.erase(iterate);
                break;
            }
        }
        clientListMutex.unlock();

        /* Set sServerAddr */
        if(inet_pton(AF_INET, sMyInfo.ipAddr.c_str(), &sServerAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerAddr.sin_port = htons(iListeningPortNum);

        /* Set sServerInfo */
        sServerInfo.name = username;
        sServerInfo.ipAddr = sMyInfo.ipAddr;
        sServerInfo.port = iListeningPortNum;

        /* Copy the contents of sentBuffer to broadcast queue */
        sentbufferMutex.lock();
        int iTempSeqNum = 0;
        for(std::map<int ,msg_struct *>::iterator it = sentBufferMap.begin();
                it != sentBufferMap.end(); ++it)
        {
            psMsgStruct = it->second;
            psMsgStruct->seqNum = iTempSeqNum++;
            qpsBroadcastq.push(psMsgStruct);
        }
        sentBufferMap.clear();
        sentbufferMutex.unlock();

        /* Set is_server to true and send NEW_LEADER_ELECTED to all clients */
        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::NEW_LEADER_ELECTED);
        strcpy(&acBuffer[NAME], username.c_str());
        strcpy(&acBuffer[DATA], sServerInfo.ipAddr.c_str());
        sprintf(&acBuffer[SENDER_LISTENING_PORT], "%d", sServerInfo.port);

        std::cout << "Sending new leader elected msg\n";

        clientListMutex.lock();
        is_server = true;
        iSeqNum = iMsgId = iExpSeqNum = 0;
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {
            sendto(iSockFd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr *) *i, sizeof(*(*i)));
        }
        clientListMutex.unlock();
    }
    else
    {
        /* Clear the lpsClients list in case it it not a server */
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {
            if(*i != NULL)
            {
                delete *i;
            }
            i = lpsClients.erase(i);
        }
    }
    newLeaderElectedMutex.unlock();
}
