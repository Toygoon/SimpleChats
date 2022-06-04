#ifndef COMMON_H
#define COMMON_H

#define BUF_SIZE 100
#define MAX_CLIENT 256
#define MAX_ROOM 10
#define NAME_SIZE 20

typedef struct _Room {
    int roomId;
    char roomName[BUF_SIZE];
} Room;

#endif