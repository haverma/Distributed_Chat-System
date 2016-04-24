dchat: executable
CC=g++

executable: broadcast_message.cpp client_chat_ack.cpp main.cpp msg_listener.cpp user_listener.cpp leader_election.cpp heartbeat.cpp
	$(CC) -g -std=c++11 -o dchat broadcast_message.cpp client_chat_ack.cpp main.cpp msg_listener.cpp user_listener.cpp heartbeat.cpp leader_election.cpp -pthread

clean: 
	rm dchat

