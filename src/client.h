/* Client headers for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

#define MAIN_WIDTH 600
#define MAIN_HEIGHT 600

#define LOGIN_WIN_WIDTH 200
#define LOGIN_WIN_HEIGHT 100

#define ROOM_WIN_WIDTH 200
#define ROOM_WIN_HEIGHT 50

static void getLoginData(GtkApplication *, gpointer);
void *receiveData(void *);
char *createMsg(char *, char *, char *);
void *connectServer(void *);
void logger(char *);
gboolean closeRequest(GtkWindow *, gpointer);
void sendText(void);
void sendRoomRequest(GtkApplication *, GtkApplication *);
void sendEnterRoomRequest(GtkApplication *, GtkApplication *);
void showRoomListWindow(char *);
void createRoomRequest(GtkApplication *, gpointer);
void enterRoomRequest(GtkApplication *, gpointer);
static gboolean buttonPressed(GtkWidget *, GdkEventKey *, gpointer);
static void mainWindow(GtkApplication *, gpointer);
static void loginWindow(GtkApplication *, gpointer);
char *gtkui_utf8_validate(char *);

GtkApplication *app;
GtkWidget *loginWin;
GtkWidget *logText;
GtkTextBuffer *logTextBuffer;
GtkWidget *inputText;
GtkWidget *inputEntries[3];

char name[NAME_SIZE] = "[DEFAULT]", msg[BUF_SIZE];
char serverIP[NAME_SIZE], clientName[NAME_SIZE];
int clientSocket;
bool roomEntered = false;
struct sockaddr_in servAddr;

pthread_mutex_t loggerMutex;
pthread_t connectThread, snd_thread, receiveThread;

struct RoomInfo *roomLinkedList = NULL;

#endif