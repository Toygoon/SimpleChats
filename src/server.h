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

gboolean closeRequest(GtkWindow *, gpointer);
void *startServer(void *);
static void getPortText(GtkApplication *, gpointer);
static void portWindow(GtkApplication *, gpointer);
void logger(char *);
void clearLogger(void);
void showMembers(void);
void newMember(int, char *);
void removeMember(int);
int findSocketByName(char *);
MemberInfo *findMemberBySocket(int);
int createNewRoom(int, char *);
void sendRoomList(int);
void enterRoomRequest(int, char *);
static void manageWindow(GtkApplication *, gpointer);
void *handleClient(void *);
void sendGlobalMsg(char *, int);
void sendRoomMsg(int, char *);
char *gtkui_utf8_validate(char *);
static gboolean buttonPressed(GtkWidget *, GdkEventKey *, gpointer);

int clientCount = 0, clientSockets[MAX_CLIENT], clientAddrSz;
pthread_mutex_t clientMutex, memberMutex, loggerMutex;

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