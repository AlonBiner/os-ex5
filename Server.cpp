#include <cstdlib>
#include <iostream>
#include "Server.h"

/**
 * Initialize a server
 */
void initServer(char *port) {
    uint16_t portNumber = strtol(port, nullptr, 10);

    int sockFD = establishConnection(portNumber);
    while (true) {
        char *buf = (char *) malloc(BUFF_SIZE * sizeof (char ));
        if (buf == nullptr) {
            std::cerr << SYS_CALL_ERR << MEM_ALLOC_FAILURE_MSG << std::endl;
            exit(EXIT_FAILURE);
        }
        int conSockFD = getConnection(sockFD);

        if (readData(conSockFD, buf, BUFF_SIZE) != 0) {
            system(buf);
        }

        if (close(conSockFD) == -1) {
            std::cerr << SYS_CALL_ERR << SOCKET_CLOSING_FAILURE_MSG << std::endl;
            exit(EXIT_FAILURE);
        }
        free(buf);
    }
}

/**
 * Creates the server socket
 */
int establishConnection(uint16_t portNumber) {
    int sockFD;
    struct sockaddr_in sa{};
    struct hostent *hp;

    hp = gethostbyname(LOCAL_HOST);
    if (hp == nullptr) {
        std::cerr << SYS_CALL_ERR << HOST_BY_NAME_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    //sockaddrr_in initialization
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = hp->h_addrtype;
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_port = htons(portNumber);

    /* create socket */
    if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << SYS_CALL_ERR << SOCKET_CREATION_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    if (bind(sockFD, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
        std::cerr << SYS_CALL_ERR << BIND_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(sockFD, MAX_CLIENTS_NUM) < 0) {
        std::cerr << SYS_CALL_ERR << LISTEN_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    return sockFD;
}

/**
 * Accepts a new client
 */
int getConnection(int sockFD) {
    int conSockFD;

    if ((conSockFD = accept(sockFD, nullptr, nullptr)) < 0) {
        std::cerr << SYS_CALL_ERR << ACCEPT_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }
    return conSockFD;
}

/**
 * Reads data from a given socket
 */
ssize_t readData(int sockFD, char *buf, int bytesToRead) {
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < bytesToRead) {
        ssize_t br = read(sockFD, buf, bytesToRead - totalBytesRead);
        if (br < 0) {
            std::cerr << SYS_CALL_ERR << READ_FAILURE_MSG << std::endl;
            exit(EXIT_FAILURE);
        } else if (br == 0) {
            break;
        }

        totalBytesRead += br;
        buf += br;
    }

    return totalBytesRead;
}
