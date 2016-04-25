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

void check_hbm_and_display();

void holdback_patrol()
{
    while(shut_down)
    {
        expSeqNumMutex.lock();
        check_hbm_and_display();
        expSeqNumMutex.unlock();
        sleep(5);
    }
}
