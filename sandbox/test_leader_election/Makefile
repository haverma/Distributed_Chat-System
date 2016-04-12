dchat: executable
CC=g++

executable: broadcast_message.cpp client_chat_ack.cpp main.cpp msg_listener.cpp user_listener.cpp
	$(CC) -g -std=c++11 -o dchat broadcast_message.cpp client_chat_ack.cpp main.cpp msg_listener.cpp user_listener.cpp client_heartbeat.cpp -pthread

clean: 
	rm dchat

