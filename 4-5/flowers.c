#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../errors.h"
#include "../message.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 1234

int flowers[40];
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

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <Multicast IP> <Multicast Port> <Server IP> <Server Port>\n",
                argv[0]);
        exit(1);
    }

    multicastIP = argv[1];
    multicastPort = atoi(argv[2]);
    serverIP = argv[3];
    serverPort = atoi(argv[4]);

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

    if ((servSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        termError("socket() failed");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(serverIP);
    servAddr.sin_port = htons(serverPort);

    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP);

    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicastRequest,
                   sizeof(multicastRequest)) < 0)
        termError("setsockopt() failed");

    msg_buf *msg = (msg_buf *)malloc(sizeof(msg_buf));

    int got = 0;

    while (1) {
        int recLen;
        memset(msg, 0, sizeof(msg_buf));

        if (got == 0) {
            int amount = rand() % 15 + 1;
            printf("Choosing %d times to wither\n", amount);

            msg->msg_type = TYPE_LOG;
            msg->author_id = AUTH_FLOWER;
            msg->target_id = TRG_W;
            msg->flower_id = amount;
            msg->flower_status = FS_BAD;

            if (sendto(servSock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&servAddr,
                       sizeof(servAddr)) != sizeof(msg_buf)) {
                termError("sendto() sent a different number of bytes than expected");
            }

            memset(msg, 0, sizeof(msg_buf));
            msg->target_id = TRG_G1;
            msg->author_id = AUTH_FLOWER;
            msg->msg_type = TYPE_DATA;

            for (int i = 0; i < amount; ++i) {
                flowers[rand() % 40] = FS_BAD;
            }

            for (int i = 0; i < 40; ++i) {
                if (flowers[i] == FS_BAD) {
                    msg->flower_id = i;
                    msg->flower_status = FS_BAD;

                    printf("Flower %d is dying\n", msg->flower_id);

                    if (sendto(servSock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&servAddr,
                               sizeof(servAddr)) != sizeof(msg_buf)) {
                        termError("sendto() sent a different number of bytes than expected");
                    }
                    usleep(100 * 1000);
                    got++;
                }
            }
        }

        while (msg->target_id != TRG_F) {
            if ((recLen = recvfrom(sock, msg, sizeof(msg_buf), 0, NULL, 0)) < 0) {
                termError("recvfrom() failed");
            }
        }

        if (recLen != 0) {
            flowers[msg->flower_id] = msg->flower_status;
            if (msg->flower_status == FS_GOOD) {
                printf("[Flowerbed] Flower %d is watered\n", msg->flower_id);

                msg->msg_type = TYPE_DATA;
                msg->author_id = AUTH_FLOWER;
                msg->target_id = TRG_W;
                msg->flower_id = msg->flower_id;
                msg->flower_status = FS_GOOD;

                if (sendto(servSock, msg, sizeof(msg_buf), 0, (struct sockaddr *)&servAddr,
                           sizeof(servAddr)) != sizeof(msg_buf)) {
                    termError("sendto() sent a different number of bytes than expected");
                }
                got--;
            } else if (msg->flower_status == FS_NOT_BAD) {
                msg->msg_type = TYPE_DATA;
                msg->author_id = AUTH_FLOWER;
                msg->target_id = TRG_W;
                msg->flower_id = msg->flower_id;
                msg->flower_status = FS_NOT_BAD;

                printf("Flower %d is being watered\n", msg->flower_id);
            }
        }
    }

    close(sock);
    close(servSock);
    exit(0);
}
