#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>


void msg_listener()
{
    memset(&sServerAddr, 0x0, sizeof(sServerAddr));
    sServerAddr.sin_family = AF_INET;
    sServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sServerAddr.sin_port = htons(iPortNum);

    while(true)
    {
        iRecLen = recvfrom(iSocketFd, acBuffer, BUFF_SIZE, 0, (struct sockaddr *) &sClientAddr, &iClientAddrLen);
