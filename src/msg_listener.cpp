#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include "./chat_system.h"

void msg_listener()
{
    int iRecLen = 0, iLenToBeSent = 0;
    char acBuffer[BUFF_SIZE] = "\0";

    while(true)
    {
        if(is_server)
        {
            iRecLen = recvfrom(iListeningSocketFd, acBuffer, BUFF_SIZE, 0,
                    (struct sockaddr *) &sRecAddr, &iRecAddrLen);
            
            if (iRecLen < 0)
            {
                fprintf(stderr, "Error while receiving from the listening socket\n");
                exit(1);
            }
        }
        else
        {
            iRecLen = recvfrom(iListeningSocketFd, acBuffer, BUFF_SIZE, 0,
                    (struct sockaddr *) &sRecAddr, &iRecAddrLen);
            
            if (iRecLen < 0)
            {
                fprintf(stderr, "Error while receiving from the listening socket\n");
                exit(1);
            }
        }

        iLenToBeSent = process_rec_msg(acBuffer);
        
        if(is_server)
        {
            sendto(iSendingSocketFd, acBuffer, iLenToBeSent, 0,
                    (struct sockaddr *) &sRecAddr, iRecAddrLen);
        }
        else
        {
            sendto(iSendingSocketFd, acBuffer, iLenToBeSent, 0,
                    (struct sockaddr *) &sRecAddr, iRecAddrLen);
        }
        fflush(stdin);
    }
}

int process_rec_msg(char * acBuffer)
{
    msg_struct msg;
    //msg.addr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
    msg.msgType = atoi(acBuffer);
    msg.addr = &sRecAddr;
    msg.name = &acBuffer[NAME];
    msg.data = &acBuffer[DATA];
    sockaddr_in * psAddr;
    msg_struct * psClientInfo, * sMsg;
    int iTempIndex = 0;

    switch(msg.msgType)
    {
        case REQ_CONNECTION:
            {
                if(is_server)
                {
                    if(!is_client_already_present())
                    {
                        /* Create the sockaddr_in struct and msg_struct struct
                         * to be inserted into the two clients list */
                        psAddr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
                        memset(psAddr, 0x0, sizeof(sockaddr_in));
                        psAddr->sin_family = AF_INET;
                        psAddr->sin_addr.s_addr = htonl(&acBuffer[DATA]);
                        iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                        psAddr->sin_port = htons(atoi(&acBuffer[iTempIndex]));

                        psClientInfo = (msg_struct *) malloc(sizeof(msg_struct));
                        memset(psClientInfo, 0x0, sizeof(msg_struct));
                        psClientInfo->name = &acBuffer[NAME];
                        psClientInfo->ipAddr = &acBuffer[DATA];
                        psClientInfo->port = atoi(&acBuffer[iTempIndex]);

                        /* Insert it into the two client lists */
                        clientListMutex.lock();
                        qpsClients.push_back(psAddr);
                        qpsClientInfo.push_back(psClientInfo);
                        clientListMutex.unlock();

                        /* Insert the updated list to the broadcast queue */
                        psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                        memset(psMsg, 0x0, sizeof(msg_struct));
            }
        case CHAT:
            {
                if(is_server)
                {
                    broadcastMutex.lock();
                }
            }
