#ifndef CHAT_SYSTEM_H
#define CHAT_SYSTEM_H

#include <mutex>
#include <string>
#include <queue>
#include <map>
#include <iostream>
#include <list>
#include <time.h>

/* The below set of enums define the type of payloads in this app */

typedef enum MessageType
{
    /* 0 to 10 */

    CHAT,                       /* When server receives a msg to be broadcasts to other clients */
    MSG,                        /* When server broadcasts msg to all clients */
    ACK,                        /* When server wants to acknowledge receipt of CHAT from client */
    REQ_CONNECTION,             /* When a request for new connection is received */
    CONNECTION_ESTABLISHED,     /* When the client gets a response from server saying the connection is est. */
    SERVER_HEARTBEAT,           /* When client sends a msg to check if server is alive */
    REQ_LEADER_ELECTION,        /* When a client requests leader election */
    STOP_LEADER_ELECTION,       /* When a higher client asks another to stop election */
    NEW_LEADER_ELECTED,         /* When a new leader is elected */
    MSG_NOT_FOUND,              /* When the message is not found in broadcast buffer */
    SERVER_INFO,                /* When a client sends server info to incoming */

    /* 11 to 15 */

    NEW_CLIENT_INFO,            /* When a new client is added to the chat system */
    CLIENT_LIST,                /* When server sends an updated client list to all clients */
    RETRIEVE_MSG,               /* When the client requests the server for a msg with particular seq num */
    CLIENT_HEARTBEAT,           /* Heart beat message sent from server to all the clients */
    CLIENT_EXITED               /* Sent to the server when a client is exited */

} messageType;

/* Use the below macros as the start index for various strings that have to be
 * stored in the buffer */


/* Defines the max size of the payload that can be sent */
#define BUFF_SIZE 500

/* Defines the index of msg type */
#define MSG_TYPE 0

/* Defines the index of sequence number of the msg */
#define SEQ_NUM 3

/* Defines the index of message ID of the msg sent by client to server.
 * Required to receive ack and verify the msg */
#define MSG_ID 9

/* Defines the index of client name (30 characters) to which the payload belongs */
#define NAME 15

/* Defines the index at which the the listening port num of sender resides */
#define SENDER_LISTENING_PORT 46

/* Defines the index of data to be sent */
#define DATA 52

/* Defines the threshold number of msgs that can be stored in the broadcast
 * buffer map */
#define BBMAP_THRESHOLD 200

/* The following structure could be used to store all the information parsed
 * from the payload by the receiver */

typedef struct msg_struct
{
    messageType msgType;        /* Msg type */
    int seqNum;                 /* Sequence number of the msg */
    int msgId;                  /* Message ID of the msg */
    std::string name;           /* Name of the client to which the msg belongs */
    std::string ipAddr;         /* IP address of the client */
    int port;                   /* Corresponding port for listening to spl msgs */
    int msgPort;                /* Port for listening to incoming chat msgs */
    int senderPort;             /* Listening port number of the sender */
    struct sockaddr_in * addr;  /* Contains the addr info of the sender */
    std::string data;           /* Data present in the payload */
    int attempts;               /* No. of attempts taken by the client to send this message to the server*/
    time_t timestamp;           /* Time when the message was tried to be sent by the client*/

} msg_struct;

extern struct sockaddr_in sListeningAddr, sMsgListeningAddr;
extern msg_struct sMyInfo;
extern struct sockaddr_in sRecAddr, sRecMsgAddr;
extern int iRecAddrLen, iRecMsgAddrLen;
extern int iListeningSocketFd, iSendingSocketFd, iListeningPortNum;
extern int iMsgListeningSocketFd, iMsgSendingSocketFd, iMsgListeningPortNum;
extern int iResponseCount;

extern std::string username;

extern bool is_server, is_server_alive, declare_leader, leader_already_declared;
extern bool connection_flag;


extern std::queue<msg_struct *> qpsBroadcastq;         /* broadcast queue */
extern std::queue<msg_struct *> qpsMsgBroadcastq;      /* Broadcast queue for msgs */
extern std::list<sockaddr_in *> lpsClients;            /* client sockaddr */
extern std::list<sockaddr_in *> lpsClientsMsg;         /* client sockaddr for msg sending */
extern std::list<msg_struct *> lpsClientInfo;          /* client str */
extern std::map<int, msg_struct *> holdbackMap;        /* holdmap as seqnum, msgsruct */
extern int iSeqNum, iExpSeqNum;                        /* server seq, expected from client */
extern int iMsgId;                                     /* client's msg id*/
extern std::map<int, msg_struct *> broadcastBufferMap; /* server broadcast queue */
extern std::map<int, msg_struct *> sentBufferMap;      /* send broadcasst msg */
extern std::list<int> liCurrentClientPort;             /* Buffer for keeping an account of client heartbeat message*/


extern std::mutex seqNumMutex;
extern std::mutex expSeqNumMutex;
extern std::mutex msgIdMutex;
extern std::mutex broadcastMutex;
extern std::mutex msgBroadcastMutex;
extern std::mutex clientListMutex;
extern std::mutex broadcastbufferMutex;
extern std::mutex sentbufferMutex;
extern std::mutex heartbeatMutex;
extern std::mutex newLeaderElectedMutex;                /* Mutex used to decide whether or not to declare itself as the leader */
extern std::mutex displayMutex;

extern msg_struct sServerInfo;
extern sockaddr_in sServerAddr, sServerMsgAddr;

#endif
