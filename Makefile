all: IMclient IMserver

IMclient: IMclient.cpp
	g++ -g -o IMclient IMclient.cpp

IMserver: IMserver.cpp
	g++ -g -lpthread -o IMserver IMserver.cpp
