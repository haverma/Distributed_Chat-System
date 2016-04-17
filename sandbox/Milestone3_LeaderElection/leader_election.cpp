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
void bullyAll();

void initiate_leader_election()
{
    printf("leader election in progress...\n");
    INTERRUPT_LEADER_ELECTION = false;
    if (true) //!is_server)
    {
        // clear the map
        
        clientListMutex.lock();
        lpsClients.erase(lpsClients.begin(), lpsClients.end());
        clientListMutex.unlock();
        
        
        // assemble client list to send to
        for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i) {
            // populating the message struct from the tokens from strtok
            
            
            printf("reassembling client list\n");
            

            if (sMyInfo.port != (*i)->port && (*i)->port != sServerInfo.port) {
                
                printf("processing port no. %d\n", (*i)->port);
                
                sockaddr_in* psAddr = new sockaddr_in; //();
                if (psAddr == NULL) {
                    fprintf(stderr, "Malloc failed. Please retry\n");
                    break;
                }
                psAddr->sin_family = AF_INET;
                inet_pton(AF_INET, (*i)->ipAddr.c_str(), &(psAddr->sin_addr));
                psAddr->sin_port = htons((*i)->port);

                // Insert it into the two client lists
                clientListMutex.lock();
                lpsClients.push_back(psAddr);
                clientListMutex.unlock();
            }
        }
        
        
        //while (true) {
            char acBuffer[BUFF_SIZE];
            int sockfd;
            // assemble STOP_LEADER_ELECTION message and send
            sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::STOP_LEADER_ELECTION);
            strcpy(&acBuffer[DATA], sMyInfo.ipAddr.c_str());
            int iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
            sprintf(&acBuffer[iTempIndex], "%d", sMyInfo.port);
            for (std::list<sockaddr_in*>::iterator i = lpsClients.begin(); i != lpsClients.end(); i++) {
                int port = ntohs(((*i))->sin_port);
                
                printf ("CLIENT PORT = %d MY PORT = %d \n", port, sMyInfo.port);
                // send message to clients with lower port numbers
                if (port < sMyInfo.port){
                    std::cout << "sending to the client " << port << "\n";
                    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
                    int n = sendto(sockfd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr*)*i, sizeof(*(*i)));
                    if (n < 0)
                        perror("ERROR in sendto");
                    //close(sockfd);
                }
                sleep(1);
            }
            
            printf("waiting----------- interrupt = %d \n", INTERRUPT_LEADER_ELECTION);
            // start timer
            /*
            double current = std::clock();
            double interval = 10; //10 seconds
            while (true)
            {
                // if we waited longer that allocated time, quit
                if( std::clock() - current  > interval * 1000000)
                    break;
            }
            */
            sleep(5);
            printf("done waiting------ interrupt = %d \n", INTERRUPT_LEADER_ELECTION);
            if (INTERRUPT_LEADER_ELECTION)
                return;
            
            
        //}
    }
    if (!INTERRUPT_LEADER_ELECTION)
        bullyAll();
    //INTERRUPT_LEADER_ELECTION = false;
        
    printf("--------------------------- is_server = %d\n", is_server);
    printf("leader election finished.\n");
    INTERRUPT_LEADER_ELECTION = false;
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

void bullyAll()
{
    printf("running logic \n");
    // declare yourself server
    deleteClientatServer(sMyInfo.port);
    sServerAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, sMyInfo.ipAddr.c_str(), &sServerAddr.sin_addr) <= 0) {
        fprintf(stderr, "Error while storing server IP address. Please retry\n");
        //break;
    }
    sServerAddr.sin_port = htons(sMyInfo.port);

    // Store server's information in the global sServerInfo struct
    sServerInfo.name = username;
    sServerInfo.ipAddr = sMyInfo.ipAddr;
    sServerInfo.port = sMyInfo.port;
    is_server = true;
    iSeqNum = 0;
    iExpSeqNum = 0;

    printf("IS SERVER: %d\n", is_server);
    // broadcast message to all

    if (is_server) {
        
        // this is already assembled no need to redo
        
        // assemble client list to send to
        /*
        for (std::list<sockaddr_in *>::iterator i = lpsClients.begin(); i != lpsClients.end(); ++i)
        {
            lpsClients.erase(i);
            delete *i;
        }
        
        for (std::list<msg_struct*>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i) {
            sockaddr_in* psAddr = new sockaddr_in; //();
            if (psAddr == NULL) {
                fprintf(stderr, "Malloc failed. Please retry\n");
                //break;
            }
            psAddr->sin_family = AF_INET;
            inet_pton(AF_INET, (*i)->ipAddr.c_str(), &(psAddr->sin_addr));
            psAddr->sin_port = htons((*i)->port);
            std::cout << "Adding port: " << ntohs(psAddr->sin_port);
            // Insert it into the two client lists
            clientListMutex.lock();
            lpsClients.push_back(psAddr);
            clientListMutex.unlock();
        }
        */
        
    
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
            std::cout << "sending to the client " << port << "\n";
            sockfd = socket(PF_INET, SOCK_DGRAM, 0);
            int n = sendto(sockfd, acBuffer, sizeof(acBuffer), 0, (struct sockaddr*)*i, sizeof(*(*i)));
            if (n < 0)
                perror("ERROR in sendto");
            close(sockfd);
        }
    }
}

#endif // __LEADER_ELECTION__
