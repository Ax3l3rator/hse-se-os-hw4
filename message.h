#ifndef MESSAGE_H
#define MESSAGE_H

// flower statuses
#define FS_GOOD 0     // Healthy flower
#define FS_BAD 1      // Withering flower
#define FS_NOT_BAD 2  // Flower is being watered

// targets of messages
#define TRG_S 0   // server
#define TRG_F 2   // flowers
#define TRG_G1 1  // gardener 1
#define TRG_G2 3  // gardener 2
#define TRG_W 4   // watcher

// authors for watcher
#define AUTH_SERVER 0
#define AUTH_GARD1 1
#define AUTH_GARD2 2
#define AUTH_FLOWER 3

#define TYPE_NET 0
#define TYPE_DATA 1
#define TYPE_LOG 2

// message structure
typedef struct UDP_MESSAGE {
    // message destination
    int target_id;
    // flower id
    int flower_id;
    // flower status
    int flower_status;
    // message author
    int author_id;
    // message type
    int msg_type;
} msg_buf;

#endif