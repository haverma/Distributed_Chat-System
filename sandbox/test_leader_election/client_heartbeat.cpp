#ifndef __CLIENT_HEARTBEAT__
#define __CLIENT_HEARTBEAT__

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

void client_heartbeat();
void flush_dead_clients(std::list<int> deadclients);

void client_heartbeat() {
    while (1){
        if (is_server){
            if (liCurrentClientPort.empty()) {
                CurrentClientsListMutex.lock();
                for (std::list<sockaddr_in *>::iterator i = std::next(lpsClients.begin()); i != lpsClients.end(); ++i) {
                    liCurrentClientPort.push_back(ntohs((*i)->sin_port));
                }
                CurrentClientsListMutex.unlock();
            } else {
                flush_dead_clients(liCurrentClientPort);
                liCurrentClientPort.clear();
                for (std::list<sockaddr_in *>::iterator i = std::next(lpsClients.begin()); i != lpsClients.end(); ++i) {
                    liCurrentClientPort.push_back(ntohs((*i)->sin_port));
                }
            }
            char buf[BUFF_SIZE];
            sprintf(&buf[MSG_TYPE], "%d", messageType::CLIENT_HEARTBEAT);
            for (std::list<sockaddr_in *>::iterator i = std::next(lpsClients.begin()); i != lpsClients.end(); ++i) {
                int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
                int n = sendto(sockfd, buf, sizeof (buf), 0, (struct sockaddr *) *i, sizeof (*(*i)));
                if (n < 0)
                    perror("ERROR in sendto");
            }


            sleep(3);
        }
    }
}

void flush_dead_clients(std::list<int> deadclients) {
    for (std::list<int>::iterator i = deadclients.begin(); i != deadclients.end(); ++i) {
        int port = (*i);
        for (std::list<msg_struct *>::iterator iter = lpsClientInfo.begin(); iter != lpsClientInfo.end(); ++iter) {
            msg_struct * psClientInfo = *iter;
            if (psClientInfo->port == port) {
                msg_struct * psMsg = new msg_struct; //();
                if (psMsg == NULL) {
                    fprintf(stderr, "Malloc failed. Please retry\n");
                    break;
                }
                char acTempStr[100] = "\0";
                psMsg->msgType = messageType::NEW_CLIENT_INFO;
                sprintf(acTempStr, "NOTICE %s left the chat or crashed", (psClientInfo->name).c_str());
                psMsg->data = acTempStr;
                broadcastMutex.lock();
                qpsBroadcastq.push(psMsg);
                broadcastMutex.unlock();
                iter = lpsClientInfo.erase(iter);
            }


        }
        for (std::list<sockaddr_in *>::iterator iter1 = lpsClients.begin(); iter1 != lpsClients.end(); ++iter1) {

            if (ntohs((*iter1)->sin_port) == port) {
                msg_struct * psMsg = new msg_struct; //();
                if (psMsg == NULL) {
                    fprintf(stderr, "Malloc failed. Please retry\n");
                    break;
                }
                iter1 = lpsClients.erase(iter1);
            }


        }
    }
}

#endif // __CLIENT_HEARTBEAT__
