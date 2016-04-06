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

void broadcast_message();
bool clear_broadcast_message(std::map<int, msg_struct *> broadcastbuffer);
std::string collect_clients_info();

void broadcast_message(){

   if(is_server)
   {
	   while(1)
	   {    char buf[BUFF_SIZE];
	   	    while(!qpsBroadcastq.empty())
	   	    {	
	   	    	broadcastMutex.lock();
	   	    	msg_struct* psMsgStruc = qpsBroadcastq.front();
	   	    	qpsBroadcastq.pop();
	   	    	broadcastMutex.unlock();

	   	    	broadcastbufferMutex.lock();
	            //mlsBroadcastm.insert ( std::pair<long,msg_struct *>(psMsgStruc->seqNum, psMsgStruc) );
	            if(broadcastBufferMap.size() == BBMAP_THRESHOLD)
	            {
	                clear_broadcast_message(broadcastBufferMap);
	            }
	            broadcastBufferMap[psMsgStruc->seqNum] = psMsgStruc;
	            broadcastbufferMutex.unlock();
	            std::string message;
	            memset((char *)&buf, 0, sizeof(buf));
	            if(psMsgStruc->msgType == CLIENT_LIST){
	                message = collect_clients_info();
	                sprintf(&buf[MSG_TYPE], "%d", psMsgStruc->msgType);
	            	sprintf(&buf[DATA], "%s", message.c_str());
	            }
	            else if(psMsgStruc->msgType == NEW_CLIENT_INFO){
	            	message = psMsgStruc->data;
	                sprintf(&buf[MSG_TYPE], "%d", psMsgStruc->msgType);
	            	sprintf(&buf[DATA], "%s", message.c_str());
	            }
	            else
	            {
	            	message = psMsgStruc->data;
	            	sprintf(&buf[MSG_TYPE], "%d", psMsgStruc->msgType);
	            	sprintf(&buf[DATA], "%s", message.c_str());
		            sprintf(&buf[NAME], "%s", psMsgStruc->name.c_str());
		            sprintf(&buf[SEQ_NUM], "%d", psMsgStruc->seqNum);
	            }
	            
	            
		        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) 
		        {
		        	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
		            int n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)*i, sizeof(*(*i)));
		            if (n < 0) 
		                perror("ERROR in sendto"); 
		        }
		    }
		    sleep(1);
	    }
    }
}

bool clear_broadcast_message(std::map<int, msg_struct *> broadcastbuffer)
{
	std::map<int,msg_struct *>::iterator it;
    
	broadcastbufferMutex.lock();
	for (it=broadcastbuffer.begin(); it!=broadcastbuffer.end(); ++it){
		msg_struct* temp = it-> second;

		if(temp != NULL)
		{
			if(temp -> addr != NULL)
			{
				free(temp -> addr);
				temp -> addr = NULL;
			}
		}
		free(temp);
		temp = NULL;
	}
	broadcastbuffer.clear();
	broadcastbufferMutex.unlock();
	return true;

}

std::string collect_clients_info()
{
	std::string message = "";
	for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
	{
        message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + std::to_string((*i)->port) + ",";
	}
	return message;
}