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
#include "leader_election.cpp"

void heartbeat();
void flush_dead_clients(std::list<int> deadclients);

void heartbeat()
{
    char buf[BUFF_SIZE];
    int n;
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    bool bListEmpty, bServerAlive;
    while(1)
    {
        /* If server, check if clients are alive */
        if(is_server)
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
	        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
	        {
	            n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) *i, sizeof(*(*i)));
	            if (n < 0)
                {
	                fprintf(stdout, "Error while sending client heartbeat msg\n");
                }
	        }
            sleep(3);
        }
        /* If client, check if server is alive */
        else
        {
            sprintf(&buf[MSG_TYPE], "%d", messageType::SERVER_HEARTBEAT);
            sprintf(&buf[SENDER_LISTENING_PORT], "%d", iListeningPortNum);
            heartbeatMutex.lock();
            is_server_alive = false;
            heartbeatMutex.unlock();
            //std::cout << "server addr in heartbeat: " << ntohs(sServerAddr.sin_port) << "\n";
            n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &sServerAddr, sizeof(sServerAddr));
            if (n < 0)
            {
                fprintf(stdout, "Error while sending server heartbeat msg\n");
            }
            sleep(3);
            heartbeatMutex.lock();
            bServerAlive = is_server_alive;
            heartbeatMutex.unlock();
            if(!bServerAlive)
            {
                heartbeatMutex.lock();
                if(0 == iResponseCount)
                {
                    iResponseCount++;
                    heartbeatMutex.unlock();
                }
                else
                {
                    iResponseCount = 0;
                    heartbeatMutex.unlock();
                    newLeaderElectedMutex.lock();
                    //std::cout << "setting leader_already_declared to false\n";
                    leader_already_declared = false;
                    newLeaderElectedMutex.unlock();
                    initiate_leader_election();
                    sleep(3);
                }
            }
        }
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
                sprintf(acTempStr, "NOTICE: %s left the chat or crashed",(psClientInfo->name).c_str());
                psMsg->data = acTempStr;
                broadcastMutex.lock();
                qpsBroadcastq.push(psMsg);
                broadcastMutex.unlock();
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
}
