all : clean dchat

dchat : main.cpp broadcast_message.cpp client_chat_ack.cpp msg_listener.cpp user_listener.cpp heartbeat.cpp leader_election.cpp spl_msg_listener.cpp holdback_patrol.cpp
	g++ main.cpp broadcast_message.cpp client_chat_ack.cpp msg_listener.cpp user_listener.cpp heartbeat.cpp leader_election.cpp spl_msg_listener.cpp holdback_patrol.cpp -std=c++11 -g -o dchat -pthread

clean :
	rm -f dchat
