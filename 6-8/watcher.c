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
void sigHandle(int dummy) {
    close(sock);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandle);
    struct sockaddr_in multicastAddr;
    struct sockaddr_in servAddr;

    char *multicastIP;
    unsigned short multicastPort;

    struct ip_mreq multicastRequest;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Multicast IP> <Multicast Port>\n", argv[0]);
        exit(1);
    }

    multicastIP = argv[1];
    multicastPort = atoi(argv[2]);

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

    msg_buf *msg = (msg_buf *)malloc(sizeof(msg_buf));

    int got = 0;

    while (1) {
        int recLen;
        memset(msg, 0, sizeof(msg_buf));

        if ((recLen = recvfrom(sock, msg, sizeof(msg_buf), 0, NULL, 0)) < 0) {
            termError("recvfrom() failed");
        }

        if (recLen != 0) {
            char *authStr;
            char contentStr[255];
            if (msg->author_id == AUTH_FLOWER) {
                authStr = "[Flowerbed]";
                if (msg->target_id != TRG_S) {
                    if (msg->msg_type == TYPE_LOG) {
                        if (msg->flower_status == FS_BAD) {
                            sprintf(contentStr, "Choosing %d times to wither", msg->flower_id);
                        }
                    } else if (msg->msg_type == TYPE_DATA) {
                        if (msg->flower_status == FS_BAD) {
                            sprintf(contentStr, "Flower %d is dying", msg->flower_id);
                        } else if ((msg->flower_status == FS_NOT_BAD)) {
                            sprintf(contentStr, "Flower %d is being watered", msg->flower_id);
                        } else if (msg->flower_status == FS_GOOD) {
                            sprintf(contentStr, "Flower %d is watered", msg->flower_id);
                        }
                    }
                }
            } else if (msg->author_id == AUTH_GARD1) {
                authStr = "[Gardener 1]";
                if (msg->msg_type == TYPE_LOG) {
                    sprintf(contentStr, "Running [Gardener %d] as [target %d]", msg->author_id,
                            msg->flower_id);
                } else if (msg->msg_type == TYPE_DATA) {
                    if (msg->flower_status == FS_NOT_BAD) {
                        sprintf(contentStr, "Watering flower %d", msg->flower_id);
                    } else if (msg->flower_status == FS_GOOD) {
                        sprintf(contentStr, "Watered flower %d", msg->flower_id);
                    }
                } else {
                }

            } else if (msg->author_id == AUTH_GARD2) {
                authStr = "[Gardener 2]";
                if (msg->msg_type == TYPE_LOG) {
                    sprintf(contentStr, "Running [Gardener %d] as [target %d]", msg->author_id,
                            msg->flower_id);
                } else if (msg->msg_type == TYPE_DATA) {
                    if (msg->flower_status == FS_NOT_BAD) {
                        sprintf(contentStr, "Watering flower %d", msg->flower_id);
                    } else if (msg->flower_status == FS_GOOD) {
                        sprintf(contentStr, "Watered flower %d", msg->flower_id);
                    }
                } else {
                }
            } else if (msg->author_id == AUTH_SERVER) {
                authStr = "[Server]";
                if (msg->msg_type == TYPE_LOG) {
                } else if (msg->msg_type == TYPE_DATA) {
                } else {
                }
            }

            printf("%s %s\n", authStr, contentStr);
        }
    }

    close(sock);
    exit(0);
}
