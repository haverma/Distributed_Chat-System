#ifndef __LEADER_ELECTION__
#define __LEADER_ELECTION__


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

static bool ELECTION_IN_PROGRESS = true;

void initiate_leader_election();
void interrupt_leader_election();

void initiate_leader_election()
{
   printf("leader election in progress...\n");
   if(!is_server)
   {
       // start timer
       std::clock_t start;
       double current = std::clock();
       double interval = 10; //10 seconds

	   while(true && ELECTION_IN_PROGRESS == true)
	   {    
            bool leader = false;
            int highest = 0;
            for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
	        //message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + std::to_string((*i)->port) + "\n";
            {
               if (highest <= (*i)->port)
                    highest = (*i)->port;
            }

            if (sMyInfo.port >= highest)
                is_server = true;
            else
                is_server = false;

            if (leader && ELECTION_IN_PROGRESS == true)
            {
                // declare yourself server
                is_server = true;

                // broadcast message to all
                
                for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) 
		        {

                    int port = ntohs(((*i))->sin_port);

                    // use ip address to check that we don't self send
                    if (sMyInfo.port != port)
                    {
                        // we have the highest port so we declare ourselves lader
                        // and broadcast message to all
                        char acBuffer[BUFF_SIZE];
                        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                
                        // assemble NEW_LEADER_ELECTED message and send
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

            // if we waited longer that allocated time, quit
            if( std::clock() - current  > interval * 1000000)
                break;
        }
    }
    // reset for next election
    ELECTION_IN_PROGRESS = true;

    printf("leader election finished.\n");
}


void interrupt_leader_election()
{
    ELECTION_IN_PROGRESS = false;
}


#endif // __LEADER_ELECTION__
