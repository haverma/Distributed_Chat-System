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
extern bool is_server;

void initiate_leader_election();
void interrupt_leader_election();

void initiate_leader_election() {
    printf("leader election in progress...\n");
    if (true) //!is_server)
    {
        // start timer
        std::clock_t start;
        double current = std::clock();
        double interval = 1; //10 seconds

        while (ELECTION_IN_PROGRESS) {
            bool leader = false;
            int highest = 0;
            for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
                //message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + std::to_string((*i)->port) + "\n";
            {
                printf("PORT COMPARE TO : %d\n", (*i)->port);
                if (highest <= (*i)->port && sMyInfo.port != (*i)->port)
                    highest = (*i)->port;
            }

            if (sMyInfo.port > highest)
                leader = true;
            else
                leader = false;

            if (ELECTION_IN_PROGRESS) {
                printf("running logic \n");
                // declare yourself server
                if (leader)
                    is_server = true;
                else
                    is_server = false;

                printf("IS SERVER: %d\n", is_server);
                // broadcast message to all

                if (is_server) {
                    // assemble client list to send to
                    for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i) {
                        // populating the message struct from the tokens from strtok
                        
                        
                            sockaddr_in* psAddr = new sockaddr_in; //();
                            if (psAddr == NULL) {
                                fprintf(stderr, "Malloc failed. Please retry\n");
                                break;
                            }
                            psAddr->sin_family = AF_INET;
                            inet_pton(AF_INET, (*i)->ipAddr.c_str(), &(psAddr->sin_addr));
                            psAddr->sin_port = htons((*i)->port);
                            std::cout<<"Adding port: " << ntohs(psAddr->sin_port);
                            /* Insert it into the two client lists */
                            clientListMutex.lock();
                            lpsClients.push_back(psAddr);
                            clientListMutex.unlock();
                        
                    }

                    for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) {
                        int port = ntohs(((*i))->sin_port);
                        
                        // use ip address to check that we don't self send
                        if (sMyInfo.port != port) {
                            //printf("current client port to send to : %d\n", port);
                            // we have the highest port so we declare ourselves lader
                            // and broadcast message to all
                            char acBuffer[BUFF_SIZE];
                            memset(acBuffer, 0x0, BUFF_SIZE * sizeof (char));

                            // assemble NEW_LEADER_ELECTED message and send
                            sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::NEW_LEADER_ELECTED);
                            strcpy(&acBuffer[DATA], sMyInfo.ipAddr.c_str());
                            int iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                            sprintf(&acBuffer[iTempIndex], "%d", sMyInfo.port);

                            int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
                            int n = sendto(sockfd, acBuffer, sizeof (acBuffer), 0, (struct sockaddr *) *i, sizeof (*(*i)));
                            if (n < 0)
                                perror("ERROR in sendto");
                        }
                    }
                    // now that we are the server we can break the loop
                    break;
                }
            }
            // if we waited longer that allocated time, quit
            //if( std::clock() - current  > interval * 1000000)
                break;
        }
    }
    // reset for next election
    ELECTION_IN_PROGRESS = true;

    printf("leader election finished.\n");
}

void interrupt_leader_election() {
    ELECTION_IN_PROGRESS = false;
}


#endif // __LEADER_ELECTION__
