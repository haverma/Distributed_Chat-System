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

int process_rec_spl_msg(char * acBuffer);
void get_msg_from_bbm(int seqNum, char * name, char * data);
void check_hbm_and_display();
void display(msg_struct * msg);
void update_client_list(msg_struct * msg);
void display_client_list();
void check_ack_sb(int time_diff_sec);
void flush_dead_clients(std::list<int> deadclients);

void print_msg_info(msg_struct * msg);

bool is_client_already_present(std::string name, std::string ip, int portNum)
{
    for (std::list<msg_struct *>::iterator it = lpsClientInfo.begin();
            it != lpsClientInfo.end(); it++)
    {
        if(((*it)->name).compare(name) == 0 &&
                ((*it)->ipAddr).compare(ip) == 0 &&
                (*it)->port == portNum)
            return true;
    }
    return false;
}

void spl_msg_listener()
{
    int iRecLen = 0, iLenToBeSent = 0;
    char acBuffer[BUFF_SIZE] = "\0";

    while(shut_down)
    {
        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
        iRecLen = recvfrom(iListeningSocketFd, acBuffer, BUFF_SIZE, 0,
                (struct sockaddr *) &sRecAddr, (socklen_t*) &iRecAddrLen);
        
        if (iRecLen < 0)
        {
            fprintf(stderr, "Error while receiving from the listening socket\n");
            exit(1);
        }

        iLenToBeSent = process_rec_spl_msg(acBuffer);

        if(iLenToBeSent != 0)
        {
            sendto(iSendingSocketFd, acBuffer, iLenToBeSent, 0,
                    (struct sockaddr *) &sRecAddr, iRecAddrLen);
        }
    }
}

int process_rec_spl_msg(char * acBuffer)
{
    msg_struct msg, * psMsg;
    msg.msgType = (messageType) atoi(acBuffer);
    msg.addr = &sRecAddr;
    msg.name = &acBuffer[NAME];
    msg.senderPort = atoi(&acBuffer[SENDER_LISTENING_PORT]);
    msg.data = &acBuffer[DATA];
    msg.seqNum = atoi(&acBuffer[SEQ_NUM]);
    msg.msgId = atoi(&acBuffer[MSG_ID]);
    sockaddr_in * psAddr, * psAddrMsg;
    msg_struct * psClientInfo, * sMsg;
    int iTempIndex = 0;
    int iLenToBeSent = 0;
    char acTempStr[100] = "\0";

    //std::cout << "SPL MSG THREAD: Received msg: \n";
    //print_msg_info(&msg);

    switch(msg.msgType)
    {
        //std::cout << "MessageType: " << msg.msgType << ", Name: " << msg.name << ", data: " << msg.data << "\n";
        case REQ_CONNECTION:
            {
                if(is_server)
                {
                    inet_ntop(AF_INET, &(sRecAddr.sin_addr), acTempStr, INET_ADDRSTRLEN);

                    if(!is_client_already_present(msg.name, std::string(acTempStr), atoi(&acBuffer[DATA])))
                    {
                        /* Create the two sockaddr_in struct and msg_struct struct
                         * to be inserted into the three clients list */
                        psAddr = new sockaddr_in;
                        psAddr->sin_family = AF_INET;
                        (psAddr->sin_addr).s_addr = sRecAddr.sin_addr.s_addr;
                        psAddr->sin_port = htons(atoi(&acBuffer[DATA]));

                        psAddrMsg = new sockaddr_in;
                        psAddrMsg->sin_family = AF_INET;
                        (psAddrMsg->sin_addr).s_addr = sRecAddr.sin_addr.s_addr;
                        iTempIndex = strlen(&acBuffer[DATA]) + DATA + 1;
                        psAddrMsg->sin_port = htons(atoi(&acBuffer[iTempIndex]));

                        psClientInfo = new msg_struct;
                        psClientInfo->name = &acBuffer[NAME];
                        psClientInfo->ipAddr = acTempStr;
                        psClientInfo->port = atoi(&acBuffer[DATA]);
                        psClientInfo->msgPort = atoi(&acBuffer[iTempIndex]);
                        
                        /* Insert it into the two client lists */
                        clientListMutex.lock();
                        lpsClients.push_back(psAddr);
                        lpsClientsMsg.push_back(psAddrMsg);
                        lpsClientInfo.push_back(psClientInfo);
                        clientListMutex.unlock();

                        /* Insert NEW_CLIENT_INFO msg to broadcast queue */
                        psMsg = new msg_struct;
                        psMsg->msgType = messageType::NEW_CLIENT_INFO;
                        sprintf(acTempStr, "NOTICE %s joined on %s:%d\n",
                            (psClientInfo->name).c_str(), psClientInfo->ipAddr.c_str(),
                            psClientInfo->port);
                        psMsg->data = acTempStr;
                        broadcastMutex.lock();
                        qpsBroadcastq.push(psMsg);
                        broadcastMutex.unlock();

                        /* Insert CLIENT_LIST msg to broadcast queue */
                        psMsg = new msg_struct;
                        psMsg->msgType = messageType::CLIENT_LIST;
                        broadcastMutex.lock();
                        qpsBroadcastq.push(psMsg);
                        broadcastMutex.unlock();

                        /* Send CONNECTION_ESTABLISHED to client */
                        sRecAddr.sin_port = htons(atoi(&acBuffer[DATA]));
                        memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                        sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::CONNECTION_ESTABLISHED);
                        strcpy(&acBuffer[NAME], username.c_str());
                        seqNumMutex.lock();
                        sprintf(&acBuffer[SEQ_NUM], "%d", iSeqNum);
                        seqNumMutex.unlock();
                        sprintf(&acBuffer[DATA], "%d", iListeningPortNum);
                        iTempIndex = strlen(&acBuffer[DATA]) + DATA + 1;
                        sprintf(&acBuffer[iTempIndex], "%d", iMsgListeningPortNum);

                        //std::cout << "Sending connection est: " << username <<
                        //    iListeningPortNum << " " << iMsgListeningPortNum << "\n";
                        iLenToBeSent = BUFF_SIZE;
                    }
                }
                else
                {
                    /* Send SERVER_INFO to client. Note that sRecAddr's port should be
                     * set to listening port of whoever sent the msg. Only
                     * then, the response will reach the client */
                    sRecAddr.sin_port = htons(atoi(&acBuffer[DATA]));
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

        case CONNECTION_ESTABLISHED:
            {
                //std::cout << "Recd connection est\n";
                //std::cout << "server: " << msg.name << " " << atoi(msg.data.c_str());
                if(!is_server)
                {
                    //std::cout << "setting flag to true\n";
                    connection_flag = true;

                    /* Set sServerInfo */
                    sServerInfo.name = msg.name;
                    inet_ntop(AF_INET, &(sRecAddr.sin_addr), acTempStr, INET_ADDRSTRLEN);
                    sServerInfo.ipAddr = acTempStr;
                    sServerInfo.port = atoi(msg.data.c_str());
                    iTempIndex = strlen(&acBuffer[DATA]) + DATA + 1;
                    sServerInfo.msgPort = atoi(&acBuffer[iTempIndex]);

                    expSeqNumMutex.lock();
                    iExpSeqNum = msg.seqNum;
                    expSeqNumMutex.unlock();

                    /* Set sServerMsgAddr */
                    sServerMsgAddr.sin_family = AF_INET;
                    sServerMsgAddr.sin_addr.s_addr = sRecAddr.sin_addr.s_addr;
                    sServerMsgAddr.sin_port = htons(sServerInfo.msgPort);

                    /* Set sServerAddr */
                    sServerAddr.sin_addr.s_addr = sRecAddr.sin_addr.s_addr;
                    sServerAddr.sin_port = htons(sServerInfo.port);

                    iLenToBeSent = 0;
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
                    strcpy(&acBuffer[NAME], username.c_str());
                    sprintf(&acBuffer[DATA], "%d", iListeningPortNum);
                    iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                    sprintf(&acBuffer[iTempIndex], "%d", iMsgListeningPortNum);
                    sRecAddr = sServerAddr;
                    iLenToBeSent = BUFF_SIZE;
                }
                break;
            }

        case NEW_CLIENT_INFO:
            {
                displayMutex.lock();
                display(&msg);
                displayMutex.unlock();
                iLenToBeSent = 0;
                break;
            }

        case CLIENT_LIST:
            {
                if(!is_server)
                {
                    update_client_list(&msg);
                }
                displayMutex.lock();
                display_client_list();
                displayMutex.unlock();
                iLenToBeSent = 0;
                break;
            }

        case CLIENT_HEARTBEAT:
            {
                if(!is_server)
                {
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::CLIENT_HEARTBEAT);
                    sprintf(&acBuffer[DATA],"%d", iListeningPortNum);
                    sRecAddr = sServerAddr;
                    iLenToBeSent = BUFF_SIZE;
                }
                else
                {
                    int iPortNo = atoi(&acBuffer[DATA]);
                    heartbeatMutex.lock();
                    liCurrentClientPort.remove(iPortNo);
                    heartbeatMutex.unlock();
                    iLenToBeSent = 0;
                }
                break;
            }

        case CLIENT_EXITED:
            {
                if(is_server)
                {
                    int port = atoi(msg.data.c_str());
                    std::list<int> liClientToBeRemoved(1, port);
                    flush_dead_clients(liClientToBeRemoved);
                    iLenToBeSent = 0;
                }
                break;
            }

        case SERVER_HEARTBEAT:
            {
                if(is_server)
                {
                    /* Send SERVER_HEARTBEAT back to the client. Note that sRecAddr's port should be
                     * set to listening port of whoever sent the msg. Only
                     * then, the response will reach the client */
                    sRecAddr.sin_port = htons(atoi(&acBuffer[SENDER_LISTENING_PORT]));
                    memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                    sprintf(&acBuffer[MSG_TYPE], "%d", (int)messageType::SERVER_HEARTBEAT);
                    iLenToBeSent = BUFF_SIZE;
                }
                else
                {
                    heartbeatMutex.lock();
                    is_server_alive = true;
                    heartbeatMutex.unlock();
                    iLenToBeSent = 0;
                }
                break;
            }

        case REQ_LEADER_ELECTION:
            {
                //std::cout << "Recd REQ_LEADER_ELECTION from: " << atoi(&acBuffer[SENDER_LISTENING_PORT]) << "\n";
                sRecAddr.sin_port = htons(atoi(&acBuffer[SENDER_LISTENING_PORT]));
                memset(acBuffer, 0x0, BUFF_SIZE * sizeof(char));
                sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::STOP_LEADER_ELECTION);
                iLenToBeSent = BUFF_SIZE;
                break;
            }

        case STOP_LEADER_ELECTION:
            {
                //std::cout << "Received STOP_LEADER_ELECTION\n";
                if(!is_server)
                {
                    heartbeatMutex.lock();
                    declare_leader = false;
                    heartbeatMutex.unlock();
                }
                iLenToBeSent = 0;
                break;
            }

        case NEW_LEADER_ELECTED:
            {
                fprintf(stdout, "NOTICE: %s left the chat or crashed\n", sServerInfo.name.c_str());
                newLeaderElectedMutex.lock();
                leader_already_declared = true;
                heartbeatMutex.lock();
                iResponseCount = 0;
                heartbeatMutex.unlock();
                //std::cout << "Recd NEW_LEADER_ELECTED msg, port:" << msg.senderPort << "\n";

                /* Store server's information in the global sServerAddr struct */
                sServerAddr.sin_family = AF_INET;
                if(inet_pton(AF_INET, msg.data.c_str(), &sServerAddr.sin_addr) <= 0)
                {
                    fprintf(stderr, "Error while storing server IP address. Please retry\n");
                    break;
                }
                sServerAddr.sin_port = htons(msg.senderPort);

                /* Store server's information in the global sServerMsgAddr struct  */
                sServerMsgAddr.sin_family = AF_INET;
                if(inet_pton(AF_INET, msg.data.c_str(), &sServerMsgAddr.sin_addr) <= 0)
                {
                    fprintf(stderr, "Error while storing server IP address. Please retry\n");
                    break;
                }
                iTempIndex = DATA + strlen(&acBuffer[DATA]) + 1;
                sServerMsgAddr.sin_port = htons(atoi(&acBuffer[iTempIndex]));

                /* Store server's information in the sServerInfo struct */
                sServerInfo.name = msg.name;
                sServerInfo.ipAddr = msg.data;
                sServerInfo.port = msg.senderPort;
                sServerInfo.msgPort = atoi(&acBuffer[iTempIndex]);

                fprintf(stdout, "NOTICE %s is the new leader\n", sServerInfo.name.c_str());

                /* Delete the new server's info from clients' list */
                clientListMutex.lock();
                for (std::list<msg_struct *>::iterator iterate = lpsClientInfo.begin(); iterate != lpsClientInfo.end(); ++iterate)
                {
                    if((*iterate)->name == sServerInfo.name && (*iterate)->port == sServerInfo.port)
                    {
                        delete *iterate;
                        lpsClientInfo.erase(iterate);
                        break;
                    }
                }
                clientListMutex.unlock();

                /* Reset the exp seq num */
                expSeqNumMutex.lock();
                iExpSeqNum = 0;
                expSeqNumMutex.unlock();
                newLeaderElectedMutex.unlock();

                sentbufferMutex.lock();
                check_ack_sb(-1);
                sentbufferMutex.unlock();
                iLenToBeSent = 0;
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

void update_client_list(msg_struct * psMessageStruct)
{
    std::list<msg_struct *> lpsTempClientList;
    std::string message = psMessageStruct->data;

    //Reading the message as a string stream 
    std::stringstream stringStream(message);
    std::string line;


    //Reading upto the new line
    while(std::getline(stringStream, line)) 
    {
        /* Copying the string into a new char pointer to change it from string to char pointer
         * and to make it char * from const char * */
        char temp[450];
        std::strcpy (temp, line.c_str());

        //char pointer array to hold name, ip , port
        char *tokens[4];
        char * token = std::strtok(temp, ":");
        int i = 0;
        while(token != NULL)
        {
            tokens[i++] = token;
            token = std::strtok(NULL, ":");
        }
        //Creating a new message structure pointer to hold one client
        msg_struct * psClientInfo = new msg_struct;

        //populating the message struct from the tokens from strtok
        psClientInfo->name = tokens[0];
        psClientInfo->ipAddr = tokens[1];
        psClientInfo->port = atoi(tokens[2]);
        psClientInfo->msgPort = atoi(tokens[3]);
        
        /* Insert it into the client info list */
        lpsTempClientList.push_back(psClientInfo);
    }

    clientListMutex.lock();
    for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
        delete (*i);

    lpsClientInfo.clear();

    for (std::list<msg_struct *>::iterator iter = lpsTempClientList.begin(); iter != lpsTempClientList.end(); ++iter)
        lpsClientInfo.push_back(*iter);
    clientListMutex.unlock();
}

void display_client_list()
{
    std::cout << "Current users:\n";
    /* Printing server's information */
    std::cout<< sServerInfo.name + " " + sServerInfo.ipAddr + ":" + std::to_string(sServerInfo.port) + " (Leader)" << "\n";

    /* Displaying clients information in a list */
    clientListMutex.lock();
    for (std::list<msg_struct *>::iterator i = lpsClientInfo.begin(); i != lpsClientInfo.end(); ++i)
    {
        msg_struct * psClientInfo = *i; 
        std::cout<< psClientInfo->name + " " + psClientInfo->ipAddr + ":" + std::to_string(psClientInfo->port)  << "\n";
    }
    clientListMutex.unlock();
    std::cout << "\n";
}
