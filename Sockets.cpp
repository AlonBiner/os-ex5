#include "Server.h"
#include "Client.h"

/**
 * Main function of the sockets program
 */
int main(int argc, char *argv[]) {

    char *port = argv[PORT_INDEX];

    if (strcmp(argv[MODE_INDEX], CLIENT_MODE) == 0) {
        initClient(port, argv[COMMAND_INDEX]);
    } else {
        initServer(port);
    }
    return 0;
}
