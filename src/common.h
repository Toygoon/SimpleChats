/* Common headers for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#ifndef COMMON_H
#define COMMON_H

#define BUF_SIZE 100
#define MAX_CLIENT 256

#define MAX_ROOM 10
#define NAME_SIZE 20

#include <arpa/inet.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct _MemberInfo {
    int socket;
    char name[NAME_SIZE];
    bool isDisabled;
    bool enteredRoom;
    struct MemberInfo* next;
} MemberInfo;

typedef struct _SocketInfo {
    int socket;
    bool isDisabled;
    struct SocketInfo* next;
} SocketInfo;

typedef struct _RoomInfo {
    char name[NAME_SIZE];
    struct SocketInfo* roomSocketList;
    struct RoomInfo* next;
} RoomInfo;

int portNum = 7777;

#endif