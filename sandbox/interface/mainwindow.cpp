#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include "../test_leader_election/chat_system.h"
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateText(QString str)
{
    ui->chatMessages->append(str);
}

void MainWindow::on_messageBox_textChanged()
{
    /*QString text = ui->messageBox->toPlainText();
    printf("%s\n", text.toStdString().c_str());

    if (text.endsWith("\n"))
    {
        text.truncate(text.length()-1);
        if (!text.startsWith("\n") && text.length() > 0)
            ui->chatMessages->append(text);
        ui->messageBox->clear();
    }*/


    char acBuffer[BUFF_SIZE] = "";
    int iTemp = 0;
    msg_struct * psMsg = NULL;
    int iSocketFd;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (iSocketFd < 0) {
        fprintf(stderr, "Error while opening socket\n");
        exit(1);
    }

    //while (true) {
        strcpy(acBuffer, "");
        iTemp = 0;

        /* Fetch the user input */

        QString text = ui->messageBox->toPlainText();
        printf("%s\n", text.toStdString().c_str());
        if (text.endsWith("\n"))
        {
            text.truncate(text.length()-1);
            if (!text.startsWith("\n") && text.length() > 0)
                ui->chatMessages->append(text);
            ui->messageBox->clear();


            strcpy(&acBuffer[DATA], text.toStdString().c_str());
            iTemp = strlen(&acBuffer[DATA]);
            acBuffer[DATA + iTemp] = '\0';
            if (!strcmp(&acBuffer[DATA], ""))
                return;


            // interface update
            ui->chatMessages->append(&acBuffer[DATA]);

            if (is_server) {
                /* Create msg by filling the received msg into a struct and push
                 * it to the broadcast queue */
                //psMsg = (msg_struct *) malloc(sizeof(msg_struct));
                psMsg = new msg_struct; //();
                if (psMsg == NULL) {
                    fprintf(stderr, "Malloc failed. Please retry\n");
                    return;
                }
                psMsg->msgType = messageType::MSG;
                seqNumMutex.lock();
                psMsg->seqNum = iSeqNum;
                iSeqNum++;
                seqNumMutex.unlock();
                psMsg->name = username;
                psMsg->data = &acBuffer[DATA];
                broadcastMutex.lock();
                qpsBroadcastq.push(psMsg);
                broadcastMutex.unlock();
            } else {
                printf("sServerPort %d\n", ntohs(sServerAddr.sin_port));
                char str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sServerAddr.sin_addr), str, INET_ADDRSTRLEN);
                printf("sServerIp %s\n", str);

                /* Store CHAT msg into acBuffer and send it to the server */
                sprintf(&acBuffer[MSG_TYPE], "%d", (int) messageType::CHAT);
                strcpy(&acBuffer[NAME], username.c_str());
                sendto(iSocketFd, acBuffer, BUFF_SIZE, 0,
                        (struct sockaddr *) &sServerAddr, sizeof (sockaddr_in));

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
                sentbufferMutex.unlock();
                iMsgId++;
            }
    }
}
