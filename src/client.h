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

char *gtkui_utf8_validate(char *);

static void loginWindow(GtkApplication *, gpointer);
static void mainWindow(GtkApplication *, gpointer);
void createRoomRequest(GtkApplication *, gpointer);
void enterRoomRequest(GtkApplication *, gpointer);
gint deleteEvent(GtkWidget *, GdkEvent *, gpointer);
void *connectServer(void *);
void getRoomList(void);
void logger(char *);
void sendText(void);
void *receiveData(void *);

GtkApplication *app;
GtkWidget *loginWin;
GtkWidget *logText;
GtkTextBuffer *logTextBuffer;
GtkWidget *inputText;
GtkWidget *inputEntries[3];

char name[NAME_SIZE] = "[DEFAULT]", msg[BUF_SIZE];
char *serverIP = "127.0.0.1", *clientName = "efgh";
int portNum = 7778, clientSocket, roomNum = -1;
struct sockaddr_in servAddr;

pthread_t connectThread, snd_thread, receiveThread;

struct RoomInfo *roomLinkedList = NULL;

#endif