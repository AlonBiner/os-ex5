#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <netdb.h>

#ifndef EX5_SERVER_H
#define EX5_SERVER_H

#define SYS_CALL_ERR "system error: "

#define READ_FAILURE_MSG "Failed to read data from client"
#define ACCEPT_FAILURE_MSG "Failed to accept client socket"
#define SOCKET_CLOSING_FAILURE_MSG "Failed to close client socket"
#define LISTEN_FAILURE_MSG "Server has failed to listen"
#define BIND_FAILURE_MSG "Server has failed to bind"
#define SOCKET_CREATION_FAILURE_MSG "Failed to create the server socket"
#define HOST_BY_NAME_FAILURE_MSG "Failed to get host by name"
#define MEM_ALLOC_FAILURE_MSG "Failed in memory allocation"

#define LOCAL_HOST "localhost"
#define MAX_CLIENTS_NUM 5
#define BUFF_SIZE 256

#define MODE_INDEX 1
#define PORT_INDEX 2
#define COMMAND_INDEX 3

#define CLIENT_MODE "client"

#endif //EX5_SERVER_H
void initServer(char *);
int establishConnection(uint16_t);
int getConnection(int);
ssize_t readData(int, char *, int);