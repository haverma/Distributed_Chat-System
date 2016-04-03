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

    switch(msg.msgType)
    {
        case CHAT:
            {
                if(is_server)
                {
                    
