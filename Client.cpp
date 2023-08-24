#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "Client.h"

/**
 * Initialize a client
 */
void initClient(char *port, char *com) {
    uint16_t portNumber = strtol(port, nullptr, 10);

    int sockFD = callSocket(LOCAL_HOST, portNumber);
    char *buf = (char *) malloc(BUFF_SIZE * sizeof(char));
    if (buf == nullptr) {
        std::cerr << SYS_CALL_ERR << MEM_ALLOC_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }
    strcpy(buf, com);
    writeData(sockFD, buf, 256);
    if (close(sockFD) == -1) {
        std::cerr << SYS_CALL_ERR << SOCKET_CLOSING_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }
    free(buf);
}

/**
 * Creates the client socket, and connects to the server socket
 */
int callSocket(const char *hostname, uint16_t portNumber) {

    struct sockaddr_in sa{};
    struct hostent *hp;
    int sockFD;

    if ((hp = gethostbyname(hostname)) == nullptr) {
        std::cerr << SYS_CALL_ERR << HOST_BY_NAME_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((u_short) portNumber);

    if ((sockFD = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        std::cerr << SYS_CALL_ERR << SOCKET_CREATION_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    if (connect(sockFD, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        std::cerr << SYS_CALL_ERR << CONNECTION_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);
    }

    return sockFD;
}

/**
 * Sends the given data to the given socket
 */
ssize_t writeData(int sockFD, char *buf, int bytesToWrite) {
    ssize_t totalBytesWritten = 0;

    while (totalBytesWritten < bytesToWrite) {
        ssize_t bw = write(sockFD, buf, bytesToWrite - totalBytesWritten);
        if (bw < 0) {
            std::cerr << SYS_CALL_ERR << WRITE_FAILURE_MSG << std::endl;
            exit(EXIT_FAILURE);
        } else if (bw == 0) {
            break;
        }

        totalBytesWritten += bw;
        buf += bw;
    }

    return totalBytesWritten;
}
