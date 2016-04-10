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

void initiate_leader_election();

void initiate_leader_election(){

   if(!is_server)
   {
	   while(1)
	   {    
            bool leader = false;
            for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
	       //message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + std::to_string((*i)->port) + "\n";
            {
               if (sMyInfo.port >= (*i)->port)
                    leader = true;
                else
                    leader = false;
            }

            if (leader)
            {
                // declare yourself server
                is_server = true;

                // broadcast message to all
                
                for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) 
		        {

                    struct in_addr ipAddr = (*i)->sin_addr; 
                    char addrStr[INET_ADDRSTRLEN];
                    inet_ntop( AF_INET, &i, addrStr, INET_ADDRSTRLEN);

                    // use ip address to check that we don't self send
                    if (strcmp(sMyInfo.ipAddr.c_str(), addrStr) != 0)
                    {
                        // we have the highest port so we declare ourselves lader
                        // and broadcast message to all
                        char acBuffer[BUFF_SIZE];
                        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                
                        //sprintf(portStr, "%d", sMyInfo.port);
                        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::NEW_LEADER_ELECTED);

                        strcpy(&acBuffer[DATA], sMyInfo.ipAddr.c_str());
                        int iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                        sprintf(&acBuffer[iTempIndex], "%d", sMyInfo.port);

    		        	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    		            int n = sendto(sockfd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr *)*i, sizeof(*(*i)));
    		            if (n < 0) 
    		                perror("ERROR in sendto"); 
	    	        }
                }
            }
	   	}
    }
}

