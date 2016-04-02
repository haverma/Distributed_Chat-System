#ifndef CHAT_SYSTEM_H
#define CHAT_SYSTEM_H

/* The below set of enums define the type of payloads in this app */

typedef enum MsgType
{
    /* 0 to 10 */

    CHAT,                       /* When server receives a msg to be broadcasts to other clients */
    MSG,                        /* When server broadcasts msg to all clients */
    ACK,                        /* When server wants to acknowledge receipt of CHAT from client */
    REQ_CONNECTION,             /* When a request for new connection is received */
    SERVER_ALIVE_CHECK,         /* When client sends a msg to check if server is alive */
    SERVER_ALIVE_RESPONSE,      /* When server responds to client saying it is alive */
    CLIENT_ALIVE_CHECK,         /* When server sends a msg to check if client is alive */
    CLIENT_ALIVE_RESPONSE,      /* When client responds to server saying it is alive */
    REQ_LEADER_ELECTION,        /* When a client requests leader election */
    STOP_LEADER_ELECTION,       /* When a higher client asks another to stop election */
    NEW_LEADER_ELECTED,         /* When a new leader is elected */

    /* 11 to 13 */

    SERVER_INFO,                /* When a client sends server info to incoming */
    CLIENT_INFO,                /* When a new client is added to the chat system */
    CLIENT_LIST                 /* When server sends an updated client list to all clients */

} msgType;


/* Use the below macros as the start index for various strings that have to be
 * stored in the buffer */


/* Defines the max size of the payload that can be sent */
#define BUFFER_SIZE 512

/* Defines the index of msg type */
#define MSG_TYPE 0

/* Defines the index of sequence number of the msg */
#define SEQ_NUM 3

/* Defines the index of message ID of the msg sent by client to server.
 * Required to receive ack and verify the msg */
#define MSG_ID 7

/* Defines the index of client name (30 characters) to which the payload belongs */
#define NAME 11

/* Defines the index of data to be sent */
#define DATA 42

#endif
