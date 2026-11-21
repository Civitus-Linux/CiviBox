#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "common.h"
#include <string.h>
#include <arpa/inet.h>

static void usage(void)
{
    xwrite("Usage: δdhcpc [OPTION] [INTERFACE]\n");
    xwrite("    -r     Release the current DHCP lease");
    xwrite("    -v     Show version information");
    xwrite("    -h     Show this help message");
}


static int create_socket(void)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0)
    {
        perror("δdhcpc: socket");
        return -1;
    }

    int broadcast = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                   &broadcast, sizeof(broadcast)) < 0)
    {
        perror("δdhcpc: SO_BROADCAST");
        close(sock);
        return -1;
    }

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(68);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("δdhcpc: bind");
        close(sock);
        return -1;
    }

    return sock;
}

int ddhcpc_cmd(int argc, char **argv)
{
    char *help = getArg(argc, argv, "-h", 0);
    if (help != NULL) usage();

    int sock = create_socket();
    if (sock < 0) return 1;
    xwrite("δdhcpc: socket initialized!");
    close(sock);

    return 0;
}
