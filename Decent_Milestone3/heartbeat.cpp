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
#include <algorithm>

void heartbeat();
void flush_dead_clients(std::list<int> deadclients);
void initiate_leader_election();
void display_client_list();

void heartbeat()
{
    char buf[BUFF_SIZE];
    int n;
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    bool bListEmpty, bServerAlive;
    while(1)
    {

        heartbeatMutex.lock();
        bListEmpty = liCurrentClientPort.empty();
        heartbeatMutex.unlock();

        if(!bListEmpty)
   	    {
            flush_dead_clients(liCurrentClientPort);
            heartbeatMutex.lock();
            liCurrentClientPort.clear();
            heartbeatMutex.unlock();
   	    }
        
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {
            liCurrentClientPort.push_back(ntohs((*i)->sin_port));
        }

        sprintf(&buf[MSG_TYPE], "%d", messageType::CLIENT_HEARTBEAT);
        sprintf(&buf[SENDER_LISTENING_PORT], "%d", iListeningPortNum);
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {
            n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) *i, sizeof(*(*i)));
            if (n < 0)
            {
                fprintf(stdout, "Error while sending client heartbeat msg\n");
            }
        }
        sleep(3);
        
        /* If client, check if server is alive */

    }
    close(sockfd);
}

void flush_dead_clients(std::list<int> deadclients)
{
    std::list<int>::iterator i = deadclients.begin();
    while(true)
    {
        heartbeatMutex.lock();
        if(i == deadclients.end())
        {
            heartbeatMutex.unlock();
            break;
        }
		int port = (*i);
        heartbeatMutex.unlock();

        std::list<msg_struct *>::iterator iter = lpsClientInfo.begin();

        /* Remove entry from lpsClientInfo */
		while(true)
    	{
            clientListMutex.lock();
            if(iter == lpsClientInfo.end())
            {
                clientListMutex.unlock();
                break;
            }
        	msg_struct * psClientInfo = *iter; 
            clientListMutex.unlock();

        	if(psClientInfo->port == port)
        	{
        		msg_struct * psMsg = new msg_struct;//();
                char acTempStr[100] = "\0";
                psMsg->msgType = messageType::NEW_CLIENT_INFO;
                std::cout << "NOTICE:"<< psClientInfo->name <<" left the chat or crashed\n";
                psMsg->data = acTempStr;
                delete psClientInfo;
                clientListMutex.lock();
                iter = lpsClientInfo.erase(iter);
                clientListMutex.unlock();
	        }
            iter++;
    	}

        std::list<sockaddr_in *>::iterator iter1 = lpsClients.begin();

        /* Remove entry from lpsClients */
    	while(true)
    	{
            clientListMutex.lock();
            if(iter1 == lpsClients.end())
            {
                clientListMutex.unlock();
                break;
            }
        	int currPort = ntohs((*iter1)->sin_port); 
            clientListMutex.unlock();
        	
            if(currPort == port)
        	{
                clientListMutex.lock();
                delete *iter1;
                iter1 = lpsClients.erase(iter1);
                clientListMutex.unlock();
	        }
            iter1++;
    	}
        i++;
	}
    msg_struct * temp = new msg_struct;//();
    if(temp == NULL)
    {
        fprintf(stderr, "Malloc failed. Please retry\n");
        exit(1);
    }
    display_client_list();
    for (std::map<int,std::list<int >>::iterator it=MapmsgId_propTS.begin(); it!=MapmsgId_propTS.end(); ++it)
    {
        if((it->second).size() == lpsClients.size()){
                std::list<int>::iterator max_it = std::max_element((it->second).begin(), (it->second).end());
                int max_ts = (*max_it);
                MapmsgId_propTS.erase(it);
                msg_struct * psMsg = new msg_struct;
                psMsg->msgType = messageType::TIMESTAMP;
                psMsg->seqNum = max_ts;
                psMsg->msgId = it->first;
                psMsg->senderPort = iListeningPortNum;
                broadcastMutex.lock();
                qpsBroadcastq.push(psMsg);
                broadcastMutex.unlock();
        }
    }
}


