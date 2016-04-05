#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include <string>
#include <list>
#include <chat_system.h>
#include <string.h>


void broadcast_message(){

   if(is_server)
   {
	   while(1)
	   {    char buf[BUFF_SIZE];
	   	    while(!qpsBroadcastq.isempty())
	   	    {	
	   	    	broadcastMutex.lock();
	   	    	msg_struct* psMsgStruc = qpsBroadcastq.pop();
	   	    	broadcastMutex.unlock();

	   	    	broadcastbufferMutex.lock();
	            //mlsBroadcastm.insert ( std::pair<long,msg_struct *>(psMsgStruc->seqNum, psMsgStruc) );
	            if(mlsBroadcastm.size() == BBMAP_THRESHOLD)
	            {
	                clear_broadcast_message(mlsBroadcastm);
	            }
	            mlsBroadcastm[psMsgStruc->seqNum] = psMsgStruc;
	            broadcastbufferMutex.unlock();
	            string message;
	            if(psMsgStruc->msgtype == CLIENT_LIST){
	                message = collect_clients_info();
	            }
	            else
	            {
	            	message = psMsgStruc->data
	            }
	            memset((char *)&buf, 0, sizeof(buf));
	            sprintf(&buf[DATA], "%s", message);
	            sprintf(&buf[NAME], "%s", psMsgStruc->name.c_str());
	            sprintf(&buf[SEQ_NUM], "%d", psMsgStruc->seqNum);
		        for (list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) 
		        {
		        	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
		            n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)*i, sizeof(*(*i));
		            if (n < 0) 
		                perror("ERROR in sendto"); 
		        }
		    }
		    sleep(1);
	    }
    }
}

bool clear_broadcast_message(std::map<long, msg_struct *> broadcastbuffer)
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

string collect_clients_info()
{
	string message = "";
	for (list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
	{
        message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + (*i)->std::to_string(port) + ",";
	}
	return string;
}