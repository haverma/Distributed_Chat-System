dchat: executable
CC=g++

executable: leader_election.cpp broadcast_message.cpp client_chat_ack.cpp main.cpp msg_listener.cpp user_listener.cpp heartbeat.cpp
	$(CC) -g -std=c++11 -o dchat leader_election.cpp broadcast_message.cpp client_chat_ack.cpp main.cpp msg_listener.cpp user_listener.cpp heartbeat.cpp -pthread

clean: 
	rm dchat

