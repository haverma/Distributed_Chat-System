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

extern bool is_server;

void initiate_leader_election();
void interrupt_leader_election();
void deleteClientatServer(int);

void initiate_leader_election()
{
    printf("leader election in progress...\n");
    if (true) //!is_server)
    {

        while (true) {
            bool leader = false;
            int highest = 0;
            for (std::list<msg_struct*>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
            //message = message + (*i)->name + ":" + (*i)->ipAddr + ":" + std::to_string((*i)->port) + "\n";
            {
                if ((*i)->port > highest) {
                    highest = (*i)->port;
                }
            }

            if (sMyInfo.port == highest)
                leader = true;
            else
                leader = false;

            printf("running logic \n");
            // declare yourself server
            if (leader) {
                deleteClientatServer(sMyInfo.port);
                sServerAddr.sin_family = AF_INET;
                if (inet_pton(AF_INET, sMyInfo.ipAddr.c_str(), &sServerAddr.sin_addr) <= 0) {
                    fprintf(stderr, "Error while storing server IP address. Please retry\n");
                    break;
                }
                sServerAddr.sin_port = htons(sMyInfo.port);

                // Store server's information in the global sServerInfo struct
                sServerInfo.name = username;
                sServerInfo.ipAddr = sMyInfo.ipAddr;
                sServerInfo.port = sMyInfo.port;
                is_server = true;
            }
            else
                is_server = false;

            printf("IS SERVER: %d\n", is_server);
            // broadcast message to all

            if (is_server) {
                // assemble client list to send to
                for (std::list<msg_struct*>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i) {
                    sockaddr_in* psAddr = new sockaddr_in; //();
                    if (psAddr == NULL) {
                        fprintf(stderr, "Malloc failed. Please retry\n");
                        break;
                    }
                    psAddr->sin_family = AF_INET;
                    inet_pton(AF_INET, (*i)->ipAddr.c_str(), &(psAddr->sin_addr));
                    psAddr->sin_port = htons((*i)->port);
                    std::cout << "Adding port: " << ntohs(psAddr->sin_port);
                    /* Insert it into the two client lists */
                    clientListMutex.lock();
                    lpsClients.push_back(psAddr);
                    clientListMutex.unlock();
                }
                char acBuffer[BUFF_SIZE];
                int sockfd;

                memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));

                // assemble NEW_LEADER_ELECTED message and send
                sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::NEW_LEADER_ELECTED);
                strcpy(&acBuffer[DATA], sMyInfo.ipAddr.c_str());
                int iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                sprintf(&acBuffer[iTempIndex], "%d", sMyInfo.port);
                for (std::list<sockaddr_in*>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i) {
                    int port = ntohs(((*i))->sin_port);
                    std::cout << "sending to the client" << port << "\n";
                    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
                    int n = sendto(sockfd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr*)*i, sizeof(*(*i)));
                    if (n < 0)
                        perror("ERROR in sendto");
                    //}
                }

              
            }
            break;
        }
    }
    printf("leader election finished.\n");
}


void deleteClientatServer(int port)
{
    clientListMutex.lock();
    for (std::list<msg_struct*>::iterator iter = lpsClientInfo.begin(); iter != lpsClientInfo.end(); ++iter) {
        msg_struct* psClientInfo = *iter;
        if (psClientInfo->port == port) {
            iter = lpsClientInfo.erase(iter);
            break;
        }
    }
    for (std::list<sockaddr_in*>::iterator iter1 = lpsClients.begin(); iter1 != lpsClients.end(); ++iter1) {

        if (ntohs((*iter1)->sin_port) == port) {
            iter1 = lpsClients.erase(iter1);
            break;
        }
    }
    clientListMutex.unlock();
}

#endif // __LEADER_ELECTION__
