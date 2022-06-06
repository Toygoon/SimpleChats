/* Server header for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define MAIN_WIDTH 600
#define MAIN_HEIGHT 600

#define PORT_WIN_WIDTH 200
#define PORT_WIN_HEIGHT 100

static void manageWindow(GtkApplication *, gpointer);
static void portWindow(GtkApplication *, gpointer);
gint deleteEvent(GtkWidget *, GdkEvent *, gpointer);
static void getPortText(GtkApplication *, gpointer);
void *handleClient(void *);
void sendMsg(char *, int);
void handleError(char *);
void *startServer(void *);
void logger(char *);
void *showMembers(void *);
void newMember(int, char *);

int clientCount = 0, totalRooms = 0;
int clientSockets[MAX_CLIENT];
pthread_mutex_t clientMutex, memberMutex;

char buf[BUF_SIZE], **roomNames;
int serverSocket;
struct sockaddr_in servAddr, clientAddr;
int clientAddrSz, portNum = 7778;
pthread_t clientsThread[MAX_CLIENT], serverThread;

GtkApplication *app;
GtkWidget *portWin;
GtkWidget *logText;
GtkTextBuffer *textBuffer;
GtkWidget *portInputEntry;

struct MemberInfo *memberLinkedList = NULL;

#endif