#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../errors.h"
#include "../message.h"

int flowers[40];
int sock;
struct sockaddr_in multicastAddr;
struct sockaddr_in recAddr;
int recSock;

void SIGIOHandler(int signalType) {
    struct sockaddr_in echoClntAddr;
    unsigned int clntLen;
    int recvMsgSize;
    msg_buf *msg = (msg_buf *)malloc(sizeof(msg_buf));

    do {
        clntLen = sizeof(echoClntAddr);
        if ((recvMsgSize = recvfrom(recSock, msg, sizeof(msg_buf), 0, NULL, 0)) < 0) {
            if (errno != EWOULDBLOCK)
                termError("recvfrom() failed");
        } else {
            if (msg->target_id & 1) {
                msg->target_id += (rand() % 2) * 2;
                if (sendto(sock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&multicastAddr,
                           sizeof(multicastAddr)) != sizeof(msg_buf)) {
                    termError("sendto() sent a different number of bytes than expected");
                }
            } else {
                if (sendto(sock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&multicastAddr,
                           sizeof(multicastAddr)) != sizeof(msg_buf)) {
                    termError("sendto() sent a different number of bytes than expected");
                }
            };
        }
    } while (recvMsgSize >= 0);
}

void die(int dummy) {
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, die);
    char *multicastIP;
    unsigned short multicastPort;
    unsigned short serverPort;
    unsigned char multicastTTL;

    struct sigaction handler;

    if ((argc < 4) || (argc > 5)) {
        fprintf(stderr,
                "Usage:  %s <Server Port> <Multicast Address> <Multicast Port> "
                "[<TTL>]\n",
                argv[0]);
        exit(1);
    }

    multicastIP = argv[2];
    multicastPort = atoi(argv[3]);
    serverPort = atoi(argv[1]);

    if (argc == 5)
        multicastTTL = atoi(argv[4]);
    else
        multicastTTL = 1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        termError("socket() failed");

    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&multicastTTL,
                   sizeof(multicastTTL)) < 0)
        termError("setsockopt() failed");

    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);
    multicastAddr.sin_port = htons(multicastPort);

    if ((recSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        termError("socket() failed");

    memset(&recAddr, 0, sizeof(recAddr));
    recAddr.sin_family = AF_INET;
    recAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    recAddr.sin_port = htons(serverPort);

    if (bind(recSock, (struct sockaddr *)&recAddr, sizeof(recAddr)) < 0)
        termError("bind() failed");

    handler.sa_handler = SIGIOHandler;

    if (sigfillset(&handler.sa_mask) < 0)
        termError("sigfillset() failed");

    handler.sa_flags = 0;

    if (sigaction(SIGIO, &handler, 0) < 0)
        termError("sigaction() failed for SIGIO");

    if (fcntl(recSock, F_SETOWN, getpid()) < 0)
        termError("Unable to set process owner to us");

    if (fcntl(recSock, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        termError("Unable to put client sock into non-blocking/async mode");

    msg_buf *msg = (msg_buf *)malloc(sizeof(msg_buf));

    for (;;) {
        printf(".\n");
        sleep(3);
    }
}
