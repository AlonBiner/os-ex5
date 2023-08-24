all: container sockets

container: container.o
	g++ container.o -o container

container.o: container.cpp
	g++ -std=c++11 -Wall -c -I. container.cpp -o container.o

sockets: Sockets.o Server.o Client.o
	g++ Sockets.o Server.o Client.o -o sockets

Sockets.o: Sockets.cpp
	g++ -std=c++11 -Wall -c -I. Sockets.cpp -o Sockets.o

Server.o: Server.h Server.cpp
	g++ -std=c++11 -Wall -c -I. Server.cpp -o Server.o

Client.o: Client.h Client.cpp
	g++ -std=c++11 -Wall -c -I. Client.cpp -o Client.o