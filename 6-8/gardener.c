#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../errors.h"
#include "../message.h"

int sock;
struct sockaddr_in multicastAddr;
struct sockaddr_in servAddr;
int servSock;

void sigHandle(int dummy) {
    close(sock);
    close(servSock);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandle);

    char *multicastIP;
    unsigned short multicastPort;
    char *serverIP;
    unsigned short serverPort;

    struct ip_mreq multicastRequest;

    if (argc != 6) {
        fprintf(stderr,
                "Usage: %s <id> <Multicast IP> <Multicast Port> <Server IP> <Server Port>\n",
                argv[0]);
        exit(1);
    }

    int id = atoi(argv[1]);

    if (id != 1 && id != 2) {
        fprintf(stderr, "Id must be 1 or 2\n");
        exit(1);
    }

    int target = atoi(argv[1]) * 2 - 1;

    printf("Running [Gardener %d] as [target %d]\n", id, target);

    multicastIP = argv[2];
    multicastPort = atoi(argv[3]);
    serverIP = argv[4];
    serverPort = atoi(argv[5]);

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        termError("socket() failed");

    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    multicastAddr.sin_port = htons(multicastPort);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        termError("setsockopt() failed");

    if (bind(sock, (struct sockaddr *)&multicastAddr, sizeof(multicastAddr)) < 0)
        termError("bind() failed");

    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP);
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicastRequest,
                   sizeof(multicastRequest)) < 0)
        termError("setsockopt() failed");

    if ((servSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        termError("socket() failed");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(serverIP);
    servAddr.sin_port = htons(serverPort);

    msg_buf *msg = (msg_buf *)malloc(sizeof(msg_buf));

    msg->author_id = id;
    msg->target_id = TRG_W;
    msg->msg_type = TYPE_LOG;
    msg->flower_id = target;

    if (sendto(servSock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&servAddr, sizeof(servAddr)) !=
        sizeof(msg_buf)) {
        termError("sendto() sent a different number of bytes than expected");
    }

    while (1) {
        int recLen;
        memset(msg, 0, sizeof(msg_buf));

        while (msg->target_id != target) {
            if ((recLen = recvfrom(sock, msg, sizeof(msg_buf), 0, NULL, 0)) < 0) {
                termError("recvfrom() failed");
            }
        }

        if (recLen != 0) {
            msg->target_id = TRG_F;
            msg->author_id = id;
            msg->flower_status = FS_NOT_BAD;

            printf("[Gardener %d] Watering flower %d\n", id, msg->flower_id);

            if (sendto(servSock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&servAddr,
                       sizeof(servAddr)) != sizeof(msg_buf)) {
                termError("sendto() sent a different number of bytes than expected");
            }

            sleep(2);
            printf("[Gardener %d] Watered flower %d\n", id, msg->flower_id);
            msg->flower_status = FS_GOOD;
            if (sendto(servSock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&servAddr,
                       sizeof(servAddr)) != sizeof(msg_buf)) {
                termError("sendto() sent a different number of bytes than expected");
            }
        }
    }

    close(sock);
    close(servSock);
    exit(0);
}
