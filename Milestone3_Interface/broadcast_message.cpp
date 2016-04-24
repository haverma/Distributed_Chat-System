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

//#include "globals.h"

void broadcast_message();
bool trim_broadcast_message(std::map<int, msg_struct *> broadcastbuffer);
std::string collect_clients_info();

void broadcast_message()
{
    while(1)
	{
        usleep(300);

        if(is_server)
   		{
            char buf[BUFF_SIZE];
            int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	   	    while(!qpsBroadcastq.empty())
	   	    {
                //picking the first element in the broadcast queue
	   	    	broadcastMutex.lock();
	   	    	msg_struct* psMsgStruct = qpsBroadcastq.front();
	   	    	qpsBroadcastq.pop();
	   	    	broadcastMutex.unlock();

	            std::string message;
	            memset((char *)&buf, 0, sizeof(buf));
	            if(psMsgStruct->msgType == CLIENT_LIST)
                {
	                message = collect_clients_info();
	                sprintf(&buf[MSG_TYPE], "%d", psMsgStruct->msgType);
	            	sprintf(&buf[DATA], "%s", message.c_str());
	            }
	            else if(psMsgStruct->msgType == NEW_CLIENT_INFO)
                {
	            	message = psMsgStruct->data;
	                sprintf(&buf[MSG_TYPE], "%d", psMsgStruct->msgType);
	            	sprintf(&buf[DATA], "%s", message.c_str());
	            }
	            else
	            {	broadcastbufferMutex.lock();
	            	//mlsBroadcastm.insert ( std::pair<long,msg_struct *>(psMsgStruct->seqNum, psMsgStruct) );
	            	if(broadcastBufferMap.size() == BBMAP_THRESHOLD)
	            	{
	                	trim_broadcast_message(broadcastBufferMap);
	            	}
	            	broadcastBufferMap[psMsgStruct->seqNum] = psMsgStruct;
	            	broadcastbufferMutex.unlock();
	            	message = psMsgStruct->data;
	            	sprintf(&buf[MSG_TYPE], "%d", psMsgStruct->msgType);
	            	sprintf(&buf[DATA], "%s", message.c_str());
		            sprintf(&buf[NAME], "%s", psMsgStruct->name.c_str());
		            sprintf(&buf[SEQ_NUM], "%d", psMsgStruct->seqNum);
	            }
	            int n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&sServerAddr, sizeof(sServerAddr));
	            if (n < 0) 
	                fprintf(stdout, "Error while sending msg to server\n");

                clientListMutex.lock();
		        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) 
		        {		        	
		            n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)*i, sizeof(*(*i)));
		            if (n < 0) 
	                fprintf(stdout, "Error while sending msg to clients\n"); 
		        }
                clientListMutex.unlock();
            }
            close(sockfd);
		    //sleep(1);
        }
    }
}

bool trim_broadcast_message(std::map<int, msg_struct *> broadcastbuffer)
{
	std::map<int,msg_struct *>::iterator it;

	broadcastbufferMutex.lock();
	for (it = broadcastbuffer.begin(); it != broadcastbuffer.end(); ++it) 
    {
	    
	    msg_struct* temp = it-> second;
	    

	    // Deleting the element and freeing the pointer to struc
		if(temp != NULL)
		{
			if(temp->addr != NULL)
			{
				//Freeing the address structure contained 
				//by the addr structure
				delete temp->addr;
				temp -> addr = NULL;
			}
		}
		delete temp;
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
        message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + std::to_string((*i)->port) + "\n";
	}
	return message;
}

