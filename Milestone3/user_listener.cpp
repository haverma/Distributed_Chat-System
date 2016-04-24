#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include "./chat_system.h"
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

/*
void delete_map(std::map<int, msg_struct *> in)
{
    std::map<int, msg_struct *>::iterator mapIterate;
    for (mapIterate = in.begin(); mapIterate != in.end(); ++mapIterate)
    {
        std::cout << "Deleting map entry\n";
        if(mapIterate->second != NULL)
            delete mapIterate->second;
        in.erase(mapIterate);
    }
    std::cout << "\n\n\n";
}

void delete_q(std::queue<msg_struct *> in)
{
    msg_struct * msg;
    while (!in.empty())
    {
        msg = in.front();
        delete msg;
        in.pop();
    }
}

void delete_sockaddr_list(std::list<sockaddr_in *> in)
{
    std::list<sockaddr_in *>::iterator sockaddrListIterate;
    for (sockaddrListIterate = in.begin(); sockaddrListIterate != in.end(); ++sockaddrListIterate)
    {
        std::cout << "Deleting list entry\n";
        delete * sockaddrListIterate;
        in.erase(sockaddrListIterate);
    }
    std::cout << "\n\n\n";
}

void delete_msg_list(std::list<msg_struct *> in)
{
    std::list<msg_struct *>::iterator msgListIterate;
    for (msgListIterate = in.begin(); msgListIterate != in.end(); ++msgListIterate)
    {
        std::cout << "Deleting list entry\n";
        delete * msgListIterate;
        in.erase(msgListIterate);
    }
    std::cout << "\n\n\n";
}
*/

void clean_up_app()
{
    for (std::map<int, msg_struct *>::iterator mapIterate = holdbackMap.begin();
            mapIterate != holdbackMap.end(); ++mapIterate)
    {
        if(mapIterate->second != NULL)
        {
            std::cout << "Deleting holdback map entry\n";
            delete mapIterate->second;
        }
        mapIterate = holdbackMap.erase(mapIterate);
    }
    for (std::map<int, msg_struct *>::iterator mapIterate = broadcastBufferMap.begin();
            mapIterate != broadcastBufferMap.end(); ++mapIterate)
    {
        if(mapIterate->second != NULL)
        {
            std::cout << "Deleting holdback map entry\n";
            delete mapIterate->second;
        }
        mapIterate = broadcastBufferMap.erase(mapIterate);
    }
    for (std::map<int, msg_struct *>::iterator mapIterate = sentBufferMap.begin();
            mapIterate != sentBufferMap.end(); ++mapIterate)
    {
        if(mapIterate->second != NULL)
        {
            std::cout << "Deleting holdback map entry\n";
            delete mapIterate->second;
        }
        mapIterate = sentBufferMap.erase(mapIterate);
    }
    for (std::list<sockaddr_in *>::iterator sockaddrListIterate = lpsClients.begin();
            sockaddrListIterate != lpsClients.end(); ++sockaddrListIterate)
    {
        std::cout << "Deleting list entry\n";
        delete * sockaddrListIterate;
        sockaddrListIterate = lpsClients.erase(sockaddrListIterate);
    }
    for (std::list<sockaddr_in *>::iterator sockaddrListIterate = lpsClientsMsg.begin();
            sockaddrListIterate != lpsClientsMsg.end(); ++sockaddrListIterate)
    {
        std::cout << "Deleting list entry\n";
        delete * sockaddrListIterate;
        sockaddrListIterate = lpsClientsMsg.erase(sockaddrListIterate);
    }
    for (std::list<msg_struct *>::iterator msgListIterate = lpsClientInfo.begin();
            msgListIterate != lpsClientInfo.end(); ++msgListIterate)
    {
        std::cout << "Deleting list entry\n";
        delete * msgListIterate;
        msgListIterate = lpsClientInfo.erase(msgListIterate);
    }
    msg_struct * msg;
    while (!qpsMsgBroadcastq.empty())
    {
        msg = qpsMsgBroadcastq.front();
        delete msg;
        qpsMsgBroadcastq.pop();
    }
    while (!qpsBroadcastq.empty())
    {
        msg = qpsBroadcastq.front();
        delete msg;
        qpsBroadcastq.pop();
    }
}

void user_listener()
{
    char acBuffer[BUFF_SIZE] = "";
    int iTemp = 0;
    msg_struct * psMsg = NULL;
    int iSocketFd;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iSocketFd < 0)
    {
        fprintf(stderr, "Error while opening socket\n");
        exit(1);
    }

    while(shut_down)
    {
        strcpy(acBuffer, "");
        iTemp = 0;
        /* Fetch the user input */
        if (fgets(&acBuffer[DATA], BUFF_SIZE - DATA - 1, stdin) == NULL)
        {
            if(!is_server)
            {
                /* Store CHAT msg into acBuffer and send it to the server */
                sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::CLIENT_EXITED);
                sprintf(&acBuffer[DATA], "%d", iListeningPortNum);
                sendto(iSocketFd, acBuffer, BUFF_SIZE, 0,
                (struct sockaddr *) &sServerAddr, sizeof(sockaddr_in));
            }
            else
            {
                fprintf(stdout, "Server exited. Leader election in progress\n");
            }
            shut_down = false;
            sleep(1);
            clean_up_app();
            exit(0);
        }

        iTemp = strlen(&acBuffer[DATA]);
        acBuffer[DATA + iTemp - 1] = '\0';
        if(!strcmp(&acBuffer[DATA], ""))
            continue;

        if(is_server)
        {
            /* Create msg by filling the received msg into a struct and push
             * it to the broadcast queue */

            psMsg = new msg_struct;
            psMsg->msgType = messageType::MSG;
            seqNumMutex.lock();
            psMsg->seqNum = iSeqNum;
            iSeqNum++;
            seqNumMutex.unlock();
            psMsg->name = username;
            psMsg->data = &acBuffer[DATA];
            msgBroadcastMutex.lock();
            qpsMsgBroadcastq.push(psMsg);
            msgBroadcastMutex.unlock();
        }
        else
        {
            /* Store CHAT msg into acBuffer and send it to the server */
            sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::CHAT);
            strcpy(&acBuffer[NAME], username.c_str());
            sprintf(&acBuffer[MSG_ID], "%d", iMsgId);
            sprintf(&acBuffer[SENDER_LISTENING_PORT], "%d", iMsgListeningPortNum);

             /* Add the message to sent buffer */
            psMsg = new msg_struct;
            psMsg->msgType = messageType::CHAT;
            psMsg->name = username;
            psMsg->data = &acBuffer[DATA];
            psMsg->msgId = iMsgId;
            psMsg->timestamp = time(NULL);
            psMsg->attempts = 1;

            sentbufferMutex.lock();
            sentBufferMap[iMsgId] = psMsg;
            iMsgId++;
            sentbufferMutex.unlock();
            //std::cout << "Msg ID sent to old server: " << iMsgId << "\n";
            sendto(iSocketFd, acBuffer, BUFF_SIZE, 0,
                    (struct sockaddr *) &sServerMsgAddr, sizeof(sockaddr_in));
        }
    }
    close(iSocketFd);
}
