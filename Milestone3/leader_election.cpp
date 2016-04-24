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


static bool deleteAll(sockaddr_in * element) 
{ 
    delete element; 
    return true; 
}

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

    /*
    for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
    {
        std::cout << "Entry in lpsclients: " << ntohs((*i)->sin_port) << "\n";
        //sendto(iSockFd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr *) *i, sizeof(*(*i)));
    }
    */

    for(std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
    {
        /* Ignore if the entry belongs to the current process */
        if((*i)->name == username && (*i)->port == iListeningPortNum)
            continue;

        /* Create sockaddr struct and add it to lpsClientsMsg */
        psAddr = new sockaddr_in;
        psAddr->sin_family = AF_INET;
        if(inet_pton(AF_INET, ((*i)->ipAddr).c_str(), &(psAddr->sin_addr)) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        psAddr->sin_port = htons((*i)->msgPort);

        lpsClientsMsg.push_back(psAddr);

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

        /* Send req leader election if port < the port of list entry */
        if((*i)->port >= iListeningPortNum)
        {
            if((*i)->port == iListeningPortNum)
            {
                if(strcmp((*i)->name.c_str(), username.c_str()) < 0)
                {
                    continue;
                }
            }
            //std::cout << "Sending REQ_LEADER_ELECTION\n";

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
        /*
        for (std::list<msg_struct *>::iterator iterate = lpsClientInfo.begin(); iterate != lpsClientInfo.end(); ++iterate)
        {
            std::cout<< "Client Info: "<< (*iterate)->port << "\n";
        }
        */
        clientListMutex.unlock();

        /* Set sServerAddr */
        if(inet_pton(AF_INET, sMyInfo.ipAddr.c_str(), &sServerAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerAddr.sin_port = htons(iListeningPortNum);

        /* Set sServerMsgAddr */
        if(inet_pton(AF_INET, sMyInfo.ipAddr.c_str(), &sServerMsgAddr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error while storing the IP address. Please retry\n");
            exit(1);
        }
        sServerMsgAddr.sin_port = htons(iMsgListeningPortNum);

        /* Set sServerInfo */
        sServerInfo.name = username;
        sServerInfo.ipAddr = sMyInfo.ipAddr;
        sServerInfo.port = iListeningPortNum;
        sServerInfo.msgPort = iMsgListeningPortNum;

        /* Copy the contents of sentBuffer to broadcast queue */
        sentbufferMutex.lock();
        int iTempSeqNum = 0;
        for(std::map<int ,msg_struct *>::iterator it = sentBufferMap.begin();
                it != sentBufferMap.end(); ++it)
        {
            psMsgStruct = it->second;
            psMsgStruct->seqNum = iTempSeqNum++;
            psMsgStruct->msgType = messageType::MSG;
            qpsMsgBroadcastq.push(psMsgStruct);
        }
        sentBufferMap.clear();
        sentbufferMutex.unlock();

        /* Set is_server to true and send NEW_LEADER_ELECTED to all clients */
        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::NEW_LEADER_ELECTED);
        strcpy(&acBuffer[NAME], username.c_str());
        strcpy(&acBuffer[DATA], sServerInfo.ipAddr.c_str());
        sprintf(&acBuffer[SENDER_LISTENING_PORT], "%d", sServerInfo.port);
        int iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
        sprintf(&acBuffer[iTempIndex], "%d", sServerInfo.msgPort);

        clientListMutex.lock();
        is_server = true;
        seqNumMutex.lock();
        iSeqNum = iTempSeqNum;
        seqNumMutex.unlock();
        iMsgId = 0;
        iExpSeqNum = 0;
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {   
            //std::cout << "Sending NEW_LEADER_ELECTED to " << ntohs((*i)->sin_port) << "\n";
            sendto(iSockFd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr *) *i, sizeof(*(*i)));
        }
        clientListMutex.unlock();
    }
    else
    {
        /* Clear lpsClientsMsg and lpsClients list in case it it not a server */
        lpsClients.remove_if(deleteAll);
        lpsClientsMsg.remove_if(deleteAll);

        /*
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {   
            std::cout << "Entry in lpsclients after deletion: " << ntohs((*i)->sin_port) << "\n";
            //sendto(iSockFd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr *) *i, sizeof(*(*i)));
        }
        */
    }
    newLeaderElectedMutex.unlock();
}
