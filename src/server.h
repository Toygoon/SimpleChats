/* Server header for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define MAIN_WIDTH 500
#define MAIN_HEIGHT 500

#define PORT_WIN_WIDTH 200
#define PORT_WIN_HEIGHT 100

char *gtkui_utf8_validate(char *);

static void manageWindow(GtkApplication *, gpointer);
static void portWindow(GtkApplication *, gpointer);
gint deleteEvent(GtkWidget *, GdkEvent *, gpointer);
static void getPortText(GtkApplication *, gpointer);

void sendGlobalMsg(char *, int);
void handleError(char *);
void logger(char *);

void *startServer(void *);
void *handleClient(void *);

void showMembers(void);
void newMember(int, char *);
void removeMember(int);
int findSocketByName(char *);
int createNewRoom(int, char *);
void sendRoomList(int);

int clientCount = 0, clientSockets[MAX_CLIENT], clientAddrSz, portNum = 7778, roomId = 0;
pthread_mutex_t clientMutex, memberMutex;

char buf[BUF_SIZE];
int serverSocket;
struct sockaddr_in servAddr, clientAddr;
pthread_t clientsThread, serverThread;

GtkApplication *app;
GtkWidget *portWin;
GtkWidget *logText;
GtkTextBuffer *textBuffer;
GtkWidget *portInputEntry;

struct MemberInfo *memberLinkedList = NULL;
struct RoomInfo *roomLinkedList = NULL;

#endif