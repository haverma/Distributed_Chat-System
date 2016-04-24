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
#include <time.h>  

void check_ack_sb(int time_diff_sec)
{
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    double diff;
    char buf[BUFF_SIZE];
    std::map <int ,msg_struct *>::iterator it;

    for (it = sentBufferMap.begin(); it != sentBufferMap.end(); ++it) 
    {
        msg_struct* temp = it-> second;
        diff = time(NULL) - temp->timestamp;
        
        if(diff > time_diff_sec)
        {
            std::string message = temp->data;
            sprintf(&buf[MSG_TYPE], "%d", temp->msgType);
            sprintf(&buf[DATA], "%s", message.c_str());
            sprintf(&buf[NAME], "%s", temp->name.c_str());
            sprintf(&buf[MSG_ID], "%d", temp->msgId);
            sprintf(&buf[SENDER_LISTENING_PORT], "%d", iMsgListeningPortNum);

            //std::cout << "Msg id resent: " << temp->msgId << "\n";

            int n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&sServerMsgAddr, sizeof(sServerMsgAddr));
            if (n < 0) 
                perror("ERROR in sendto");
            //sentbufferMutex.lock();
            temp->timestamp = time(NULL);
            temp->attempts++;
            //sentbufferMutex.unlock();
        }
    }
    close(sockfd);
}
