#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Server.h"

#ifndef EX5_CLIENT_H
#define EX5_CLIENT_H

#define CONNECTION_FAILURE_MSG "Failed to connect to server"
#define WRITE_FAILURE_MSG "Failed in writing data to server"

#endif //EX5_CLIENT_H

void initClient(char *, char *);
int callSocket(const char *, unsigned short);
ssize_t writeData(int, char *, int);