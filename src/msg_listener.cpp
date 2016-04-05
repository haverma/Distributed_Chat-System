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

int process_rec_msg(char * acBuffer);
void get_msg_from_bbm(int seqNum, char * name, char * data);
void check_hbm_and_display();
void display(msg_struct * msg);

bool is_client_already_present(std::string name)
{
    for (std::list<msg_struct *>::iterator it = lpsClientInfo.begin();
            it != lpsClientInfo.end(); it++)
    {
        if(((*it)->name).compare(name) == 0)
            return true;
    }
    return false;
}

void msg_listener()
{
    int iRecLen = 0, iLenToBeSent = 0;
    char acBuffer[BUFF_SIZE] = "\0";

    while(true)
    {
        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
        if(is_server)
        {
            iRecLen = recvfrom(iListeningSocketFd, acBuffer, BUFF_SIZE, 0,
                    (struct sockaddr *) &sRecAddr, (socklen_t*) &iRecAddrLen);
            
            if (iRecLen < 0)
            {
                fprintf(stderr, "Error while receiving from the listening socket\n");
                exit(1);
            }
        }
        else
        {
            iRecLen = recvfrom(iListeningSocketFd, acBuffer, BUFF_SIZE, 0,
                    (struct sockaddr *) &sRecAddr, (socklen_t*) &iRecAddrLen);
            
            if (iRecLen < 0)
            {
                fprintf(stderr, "Error while receiving from the listening socket\n");
                exit(1);
            }
        }

        iLenToBeSent = process_rec_msg(acBuffer);
        
        if(iLenToBeSent != 0)
        {
            sendto(iSendingSocketFd, acBuffer, iLenToBeSent, 0,
                    (struct sockaddr *) &sRecAddr, iRecAddrLen);
        }

        fflush(stdin);
    }
}

int process_rec_msg(char * acBuffer)
{
    msg_struct msg, * psMsg;
    memset(&msg, 0x0, sizeof(msg_struct));
    //msg.addr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
    msg.msgType = (messageType) atoi(acBuffer);
    msg.addr = &sRecAddr;
    msg.name = &acBuffer[NAME];
    msg.data = &acBuffer[DATA];
    msg.seqNum = atoi(&acBuffer[SEQ_NUM]);
    msg.msgId = atoi(&acBuffer[MSG_ID]);
    sockaddr_in * psAddr;
    msg_struct * psClientInfo, * sMsg;
    int iTempIndex = 0;
    int iLenToBeSent = 0;
    char acTempStr[100] = "\0";

    switch(msg.msgType)
    {
        case REQ_CONNECTION:
            {
                if(is_server)
                {
                    if(!is_client_already_present(msg.name))
                    {
                        /* Create the sockaddr_in struct and msg_struct struct
                         * to be inserted into the two clients list */
                        psAddr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
                        if(psAddr == NULL)
                        {
                            fprintf(stderr, "Malloc failed. Please retry\n");
                            break;
                        }
                        memset(psAddr, 0x0, sizeof(sockaddr_in));
                        psAddr->sin_family = AF_INET;
                        (psAddr->sin_addr).s_addr = sRecAddr.sin_addr.s_addr;
                        psAddr->sin_port = htons(atoi(&acBuffer[DATA]));

                        psClientInfo = (msg_struct *) malloc(sizeof(msg_struct));
                        if(psClientInfo == NULL)
                        {
                            fprintf(stderr, "Malloc failed. Please retry\n");
                            break;
                        }
                        memset(psClientInfo, 0x0, sizeof(msg_struct));
                        psClientInfo->name = &acBuffer[NAME];
                        inet_ntop(AF_INET, &(sRecAddr.sin_addr), acTempStr, INET_ADDRSTRLEN);
                        psClientInfo->ipAddr = acTempStr;
                        psClientInfo->port = atoi(&acBuffer[DATA]);

                        /* Insert it into the two client lists */
                        clientListMutex.lock();
                        lpsClients.push_back(psAddr);
                        lpsClientInfo.push_back(psClientInfo);
                        clientListMutex.unlock();

                        /* Insert NEW_CLIENT_INFO msg to broadcast queue */
                        psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                        if(psMsg == NULL)
                        {
                            fprintf(stderr, "Malloc failed. Please retry\n");
                            break;
                        }
                        memset(psMsg, 0x0, sizeof(msg_struct));
                        psMsg->msgType = messageType::NEW_CLIENT_INFO;
                        sprintf(acTempStr, "%s joined a new chat on %s:%d, listening\
                                 on %s:%d\n", (psClientInfo->name).c_str(), sServerInfo.ipAddr.c_str(),
                                 sServerInfo.port, psClientInfo->ipAddr.c_str(), psClientInfo->port);
                        psMsg->data = acTempStr;
                        broadcastMutex.lock();
                        qpsBroadcastq.push(psMsg);
                        broadcastMutex.unlock();

                        /* Insert CLIENT_LIST msg to broadcast queue */
                        psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                        if(psMsg == NULL)
                        {
                            fprintf(stderr, "Malloc failed. Please retry\n");
                            break;
                        }
                        memset(psMsg, 0x0, sizeof(msg_struct));
                        psMsg->msgType = messageType::CLIENT_LIST;
                        broadcastMutex.lock();
                        qpsBroadcastq.push(psMsg);
                        broadcastMutex.unlock();
                        iLenToBeSent = 0;
                    }
                }
                else
                {
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::SERVER_INFO);
                    strcpy(&acBuffer[NAME], sServerInfo.name.c_str());
                    strcpy(&acBuffer[DATA], sServerInfo.ipAddr.c_str());
                    iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                    sprintf(&acBuffer[iTempIndex], "%d", sServerInfo.port);
                    iLenToBeSent = BUFF_SIZE;
                }
                break;
            }

        case SERVER_INFO:
            {
                if(!is_server)
                {
                    /* Parse server's IP and port from the DATA part of buffer
                     * and store it in the temp msg struct */
                    msg.ipAddr = &acBuffer[DATA];
                    iTempIndex = strlen(&acBuffer[DATA]) + DATA + 1;
                    msg.port = atoi(&acBuffer[iTempIndex]);

                    /* Store server's information in the global sServerAddr struct */
                    memset(&sServerAddr, 0x0, sizeof(sServerAddr));
                    sServerAddr.sin_family = AF_INET;
                    if(inet_pton(AF_INET, msg.ipAddr.c_str(), &sServerAddr.sin_addr) <= 0)
                    {
                        fprintf(stderr, "Error while storing server IP address. Please retry\n");
                        break;
                    }
                    sServerAddr.sin_port = htons(msg.port);

                    /* Store server's information in the global sServerInfo struct */
                    sServerInfo.name = msg.name;
                    sServerInfo.ipAddr = msg.ipAddr;
                    sServerInfo.port = msg.port;

                    /* Send REQ_CONNECTION to server now */
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::REQ_CONNECTION);
                    strcpy(&acBuffer[NAME], msg.name.c_str());
                    sprintf(&acBuffer[DATA], "%d", msg.port);
                    iLenToBeSent = BUFF_SIZE;
                }
                break;
            }

        case CHAT:
            {
                if(is_server)
                {
                    /* Fill msg struct with data to be sent */
                    psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                    if(psMsg == NULL)
                    {
                        fprintf(stderr, "Malloc failed. Please retry\n");
                        break;
                    }
                    memset(psMsg, 0x0, sizeof(msg_struct));
                    psMsg->msgType = messageType::MSG;
                    psMsg->seqNum = iSeqNum;
                    iSeqNum++;
                    psMsg->name = msg.name;
                    psMsg->data = msg.data;

                    /* Push the msg to the broadcast queue */
                    broadcastMutex.lock();
                    qpsBroadcastq.push(psMsg);
                    broadcastMutex.unlock();
                    iLenToBeSent = 0;
                }
                break;
            }

        case MSG:
            {
                if(!is_server)
                {
                    if(msg.seqNum == iSeqNum)
                    {
                        /* Display msg and then display all other msgs from the hbm
                         * that should be displayed */
                        /* TODO Implement the below function */
                        display(&msg);
                        iSeqNum++;

                        /* TODO: Implement the below function */
                        check_hbm_and_display();
                    }
                    else if(msg.seqNum > iSeqNum)
                    {
                        /* Create an entry in bbm for the msg */
                        psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                        if(psMsg == NULL)
                        {
                            fprintf(stderr, "Malloc failed. Please retry\n");
                            break;
                        }
                        memset(psMsg, 0x0, sizeof(msg_struct));
                        psMsg->msgType = messageType::MSG;
                        psMsg->seqNum = msg.seqNum;
                        psMsg->name = msg.name;
                        psMsg->data = msg.data;
                        holdbackMap.insert(std::pair<int, msg_struct *>(msg.seqNum, psMsg));
                    }
                    else
                    {
                        /* Control shouldn't come here */
                        fprintf(stderr, "Received unexpected msg\n");
                        break;
                    }
                }
                iLenToBeSent = 0;
                break;
            }

        case ACK:
            {
                if(!is_server)
                {
                    /* Remove entry from sent buffer */
                    sentbufferMutex.lock();
                    sentBufferMap.erase(msg.msgId);
                    sentbufferMutex.unlock();
                }
                iLenToBeSent = 0;
                break;
            }

        case NEW_CLIENT_INFO:
            {
                /* TODO: Display the msg */
                display(&msg);
                iLenToBeSent = 0;
                break;
            }

        case RETRIEVE_MSG:
            {
                if(is_server)
                {
                    /* Retrieve msg from the broadcast buffer and send that
                     * msg back to the client */
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    /* TODO: Implement the below function */
                    get_msg_from_bbm(msg.seqNum, &acBuffer[NAME], &acBuffer[DATA]);
                    sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::MSG);
                    iLenToBeSent = BUFF_SIZE;
                }
                break;
            }
    }
    return iLenToBeSent;
}

