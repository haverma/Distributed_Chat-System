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

    while(true)
    {
        strcpy(acBuffer, "");
        iTemp = 0;
        /* Fetch the user input */
        if (fgets(&acBuffer[DATA], BUFF_SIZE - DATA - 1, stdin) == NULL)
        {
            /* Store CHAT msg into acBuffer and send it to the server */
            sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::CLIENT_EXITED);
            sprintf(&acBuffer[DATA], "%d", iListeningPortNum);
            sendto(iSocketFd, acBuffer, BUFF_SIZE, 0,
            (struct sockaddr *) &sServerAddr, sizeof(sockaddr_in));
            exit(1);                
        }
        iTemp = strlen(&acBuffer[DATA]);
        acBuffer[DATA + iTemp - 1] = '\0';
        if(!strcmp(&acBuffer[DATA], ""))
            continue;
        /* Create msg by filling the received msg into a struct and push
         * it to the broadcast queue */
        //psMsg = (msg_struct *) malloc(sizeof(msg_struct));
        psMsg = new msg_struct;//();
        if(psMsg == NULL)
        {
            fprintf(stderr, "Malloc failed. Please retry\n");
            continue;
        }
        psMsg->msgType = messageType::MSG;
        //seqNumMutex.lock();
        psMsg->seqNum = 32760;
        //iSeqNum++;
        //seqNumMutex.unlock();
        psMsg->name = username;
        psMsg->data = &acBuffer[DATA];
        psMsg->msgId = iMsgId;
        iMsgId++;
        broadcastMutex.lock();
        qpsBroadcastq.push(psMsg);
        broadcastMutex.unlock();
        
   
    }
}