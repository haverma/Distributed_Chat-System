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
#include <sstream>
#include <cstring>
#include <unistd.h>

int process_rec_msg(char * acBuffer);
void get_msg_from_bbm(int seqNum, char * name, char * data);
void check_hbm_and_display();
void display(msg_struct * msg);
void update_client_list(msg_struct * msg);
void display_client_list();
void check_ack_sb(int time_diff_sec);
void flush_dead_clients(std::list<int> deadclients);

void print_msg_info(msg_struct * msg)
{
    std::cout << "Msg type: " << msg->msgType << "\n";
    //std::cout << "Seq num: " << msg->seqNum << "\n";
    //std::cout << "Msg ID: " << msg->msgId << "\n";
    //std::cout << "Name: " << msg->name << "\n";
    //std::cout << "IP addr: " << msg->ipAddr << "\n";
    //std::cout << "Port: " << msg->port << "\n";
    //std::cout << "Msg port: " << msg->msgPort << "\n";
    //std::cout << "Sender port: " << msg->senderPort << "\n";
    //std::cout << "Msg data: " << msg->data << "\n";
}

void msg_listener()
{
    int iRecLen = 0, iLenToBeSent = 0;
    char acBuffer[BUFF_SIZE] = "\0";

    while(shut_down)
    {
        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
        iRecLen = recvfrom(iMsgListeningSocketFd, acBuffer, BUFF_SIZE, 0,
                (struct sockaddr *) &sRecMsgAddr, (socklen_t*) &iRecMsgAddrLen);
        
        if (iRecLen < 0)
        {
            fprintf(stderr, "Error while receiving from the listening socket\n");
            exit(1);
        }

        iLenToBeSent = process_rec_msg(acBuffer);

        if(iLenToBeSent != 0)
        {
            sendto(iMsgSendingSocketFd, acBuffer, iLenToBeSent, 0,
                    (struct sockaddr *) &sRecMsgAddr, iRecMsgAddrLen);
        }
    }
}

int process_rec_msg(char * acBuffer)
{
    msg_struct msg, * psMsg;
    msg.msgType = (messageType) atoi(acBuffer);
    msg.addr = &sRecMsgAddr;
    msg.name = &acBuffer[NAME];
    msg.senderPort = atoi(&acBuffer[SENDER_LISTENING_PORT]);
    msg.data = &acBuffer[DATA];
    msg.seqNum = atoi(&acBuffer[SEQ_NUM]);
    msg.msgId = atoi(&acBuffer[MSG_ID]);
    sockaddr_in * psAddr;
    msg_struct * psClientInfo, * sMsg;
    int iTempIndex = 0;
    int iLenToBeSent = 0;
    char acTempStr[100] = "\0";
    //std::cout << "\n\nTEXT MSG THREAD: Received msg: \n";
    //print_msg_info(&msg);
    switch(msg.msgType)
    {
        //std::cout << "MessageType: " << msg.msgType << ", Name: " << msg.name << ", data: " << msg.data << "\n";
        case CHAT:
            {
                //std::cout << "Received CHAT:" << msg.msgId << " " << msg.senderPort << "\n";
                if(is_server)
                {
                    /* Fill msg struct with data to be sent */
                    //psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                    psMsg = new msg_struct;
                    psMsg->msgType = messageType::MSG;
                    seqNumMutex.lock();
                    psMsg->seqNum = iSeqNum;
                    iSeqNum++;
                    seqNumMutex.unlock();
                    psMsg->name = msg.name;
                    psMsg->data = msg.data;

                    /* Push the msg to the broadcast queue */
                    msgBroadcastMutex.lock();
                    qpsMsgBroadcastq.push(psMsg);
                    msgBroadcastMutex.unlock();

                    /* Send ACK to client. Note that sRecMsgAddr's port should be
                     * set to listening port of whoever sent the msg. Only
                     * then, the response will reach the client */
                    sRecMsgAddr.sin_port = htons(atoi(&acBuffer[SENDER_LISTENING_PORT]));
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::ACK);
                    sprintf(&acBuffer[MSG_ID], "%d", msg.msgId);
                    iLenToBeSent = BUFF_SIZE;
                    usleep(3000);
                }
                break;
            }

        case MSG:
            {
                //std::cout << "recd MSG: DATA: " << msg.data << " Name: " << msg.name << " Seq num: " << msg.seqNum << "\n";
                expSeqNumMutex.lock();
                if(msg.seqNum == iExpSeqNum)
                {
                    /* Display msg and then display all other msgs from the hbm
                     * that should be displayed */
                    displayMutex.lock();
                    display(&msg);
                    displayMutex.unlock();
                    iExpSeqNum++;
                    iLenToBeSent = 0;
                    check_hbm_and_display();
                }
                else if(msg.seqNum > iExpSeqNum)
                {
                    /* Create an entry in bbm for the msg */
                    //psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                    psMsg = new msg_struct;//();
                    psMsg->msgType = messageType::MSG;
                    psMsg->seqNum = msg.seqNum;
                    psMsg->name = msg.name;
                    psMsg->data = msg.data;
                    holdbackMap.insert(std::pair<int, msg_struct *>(msg.seqNum, psMsg));

                    if(msg.seqNum >= iExpSeqNum + 2)
                    {
                        for(int iterator = iExpSeqNum; iterator <= msg.seqNum; iterator++)
                        {
                            if (holdbackMap.find(iterator) == holdbackMap.end())
                            {
                                memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                                sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::RETRIEVE_MSG);
                                sprintf(&acBuffer[SENDER_LISTENING_PORT], "%d", iMsgListeningPortNum);
                                sprintf(&acBuffer[SEQ_NUM],"%d", iterator);
                                sRecMsgAddr = sServerMsgAddr;
                                iLenToBeSent = BUFF_SIZE;
                                int n = sendto(iMsgSendingSocketFd, acBuffer, iLenToBeSent, 0,(struct sockaddr *) &sRecMsgAddr, iRecAddrLen);
                            }
                        }
                        iLenToBeSent = 0;
                    }
                    else
                    {
                        iLenToBeSent = 0;
                    }
                }
                expSeqNumMutex.unlock();
                break;
            }

        case ACK:
            {
                if(!is_server)
                {
                    /* Remove entry from sent buffer */
                    std::map<int, msg_struct *>::iterator iterate =
                                sentBufferMap.find(msg.msgId);
                    sentbufferMutex.lock();
                    if (iterate != sentBufferMap.end())
                    {
                        delete iterate->second;
                        sentBufferMap.erase(msg.msgId);
                    }
                    else
                    {
                        //fprintf(stderr, "Unexpected ack received\n");
                        sentbufferMutex.unlock();
                        break;
                    }
                    if(sentBufferMap.size() > 0){
                        check_ack_sb(3);
                    }
                    sentbufferMutex.unlock();
                }
                iLenToBeSent = 0;
                break;
            }

        case RETRIEVE_MSG:
            {
                if(is_server)
                {
                    /* Retrieve msg from the broadcast buffer and send that
                     * msg back to the client. Note that sRecMsgAddr's port should be
                     * set to listening port of whoever sent the msg. Only
                     * then, the response will reach the client */
                    sRecMsgAddr.sin_port = htons(atoi(&acBuffer[SENDER_LISTENING_PORT]));
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    sprintf(&acBuffer[SEQ_NUM], "%d",msg.seqNum);
                    get_msg_from_bbm(msg.seqNum, &acBuffer[NAME], &acBuffer[DATA]);

                    /*if the sequence number is not found in the buffer map 
                    *tell the client to increase the sequence number*/
                    if(strcmp(&acBuffer[NAME], "") == 0 && strcmp(&acBuffer[DATA], "") == 0)
                    {
                        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::MSG_NOT_FOUND);
                    }
                    else
                    {
                        //When the message is found in the buffer
                        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::MSG);
                    }
                    iLenToBeSent = BUFF_SIZE;
                }
                break;
            }

        case MSG_NOT_FOUND:
            {
                expSeqNumMutex.lock();
                iExpSeqNum++;
                expSeqNumMutex.unlock();
                iLenToBeSent = 0;
                check_hbm_and_display();
                break;

            }

        default:
            {
                //fprintf(stdout, "Unexpected msg received\n", (int) msg.msgType);
                iLenToBeSent = 0;
                break;
            }            
    }
    return iLenToBeSent;
}

void get_msg_from_bbm(int seq_number, char * name, char * data)
{
    /*Finding the message from the buffer storing it in the
    *name and data arrays of acbuffer*/
    std::map<int, msg_struct *>::iterator buffer_map_iterator;

    broadcastbufferMutex.lock();
    buffer_map_iterator = broadcastBufferMap.find(seq_number);
    
    /*If found the corresponding sequence number as a
    *key in the broadcast buffer map*/
    if (buffer_map_iterator != broadcastBufferMap.end())
    {
        /*Storing the message structure in a
        temp variable the structure*/
        msg_struct * temp = buffer_map_iterator -> second;
        strcpy(name, temp->name.c_str());
        strcpy(data, temp->data.c_str());
    }
    broadcastbufferMutex.unlock();
}

void display(msg_struct * message)
{
    /*If the message type is a simple message*/
    if(message->msgType == messageType::MSG)
    {
        std::cout << message->name + ": " + message->data << "\n";
    }
    else if(message->msgType == messageType::NEW_CLIENT_INFO) //if the message is a new client notification
    {
        std::cout << message->data << "\n";
    }
}

void check_hbm_and_display()
{
    std::map<int, msg_struct *>::iterator hb_map_iterator;

    while(true)
    {
        hb_map_iterator = holdbackMap.find(iExpSeqNum);
        
        /*If found the corresponding sequence number as a
        *key in the Holdback map*/
        if (hb_map_iterator != holdbackMap.end())
        {
            /* display the message */
            displayMutex.lock();
            display(hb_map_iterator->second);
            displayMutex.unlock();
            if(hb_map_iterator->second != NULL)
            {
                delete hb_map_iterator->second;
                holdbackMap.erase(hb_map_iterator);
            }
            iExpSeqNum++;
        }
        else        //if the next sequence number is not in the holdback map
        {
            break;
        }
    }
}
