#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include <string>
#include <list>
#include <chat_system.h>


void broadcast_message(){

   while(1)
   { 
   	    while(!qpsBroadcastq.isempty())
   	    {	
   	    	msg_struct message_struc = qpsBroadcastq.pop();
            mymap.insert ( std::pair<char,int>('a',100) );
	        for (list<int>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) 
	        {
	            n = sendto(sockfd, buf, , 0, (struct sockaddr *)&serveraddr, serverlen);
	            if (n < 0) 
	                perror("ERROR in sendto"); 
	        }
	    }
}
}
