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

/* MemberInfo : 멤버 정보를 저장하는 구조체 */
typedef struct _MemberInfo {
    // 소켓 번호
    int socket;
    // 멤버 이름
    char name[NAME_SIZE];
    // 비활성화 여부
    bool isDisabled;
    // 방에 입장했는지 여부
    bool enteredRoom;
    // LinkedList처럼 다음 노드를 가짐
    struct MemberInfo* next;
} MemberInfo;

/* SocketInfo : 방에서 소켓을 저장하기 위한 구조체 */
typedef struct _SocketInfo {
    // 소켓 번호
    int socket;
    // 비활성화 여부
    bool isDisabled;
    // LinkedList처럼 다음 노드를 가짐
    struct SocketInfo* next;
} SocketInfo;

/* RoomInfo : 방 정보를 저장하기 위한 구조체 */
typedef struct _RoomInfo {
    // 방 이름
    char name[NAME_SIZE];
    // 소켓 정보를 담는 LinkedList
    struct SocketInfo* roomSocketList;
    // 방 정보를 담는 LinkedList
    struct RoomInfo* next;
} RoomInfo;

int portNum = 7777;

#endif