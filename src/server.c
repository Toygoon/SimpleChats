/* Server for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include <arpa/inet.h>
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

MemberInfo memberInfo[MAX_CLIENT];

int main(int argc, char **argv) {
    app = gtk_application_new("yu.server.simplechat", G_APPLICATION_FLAGS_NONE);

    // g_signal_connect(app, "activate", G_CALLBACK(portWindow), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(manageWindow), NULL);
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return 0;
}

gboolean closeRequest(GtkWindow *window, gpointer user_data) {
    close(serverSocket);
    gtk_window_close(window);

    return FALSE;
}

void *startServer(void *arg) {
    pthread_mutex_init(&clientMutex, NULL);
    logger("[INFO] Initiating server socket.\n");
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    logger("[INFO] Generating socket address struct.\n");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    char tmp[BUF_SIZE] = "[INFO] Setting a port number to ";
    /*
    strcat(tmp, gtk_entry_buffer_get_text(gtk_entry_get_buffer((GtkEntry*)portInputEntry)));
    strcat(tmp, "\n");
    logger(tmp);
    */
    servAddr.sin_port = htons(portNum);

    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
        logger("[ERROR] bind error, choose another port.\n");
        return FALSE;
    }

    if (listen(serverSocket, 5) == -1) {
        logger("[ERROR] listen error, please free up your memory.\n");
        return FALSE;
    }

    logger("[INFO] Server has started.\n");

    while (1) {
        clientAddrSz = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSz);

        pthread_mutex_lock(&clientMutex);
        clientSockets[clientCount] = clientSocket;
        pthread_mutex_unlock(&clientMutex);

        g_print("new socket : %d\n", clientSocket);

        pthread_create(&clientsThread[clientCount], NULL, handleClient, (void *)&clientSocket);
        pthread_detach(clientsThread[clientCount]);
        clientCount++;
        char tmp[BUF_SIZE] = "[CLIENT] Connected client IP: ";

        strcat(tmp, inet_ntoa(clientAddr.sin_addr));
        strcat(tmp, "\n");
        logger(tmp);
    }
}

static void getPortText(GtkApplication *_app, gpointer user_data) {
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)portInputEntry);
    const char *text = gtk_entry_buffer_get_text(entryBuffer);
    portNum = atoi(text);

    g_print("Port number has set to %d\n", portNum);
    manageWindow(app, NULL);
}

static void portWindow(GtkApplication *app, gpointer user_data) {
    GtkWidget *button;
    GtkWidget *portInputLabel;
    GtkWidget *grid;
    gchar *text;

    portWin = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(portWin), "Port");
    gtk_window_set_default_size(GTK_WINDOW(portWin), PORT_WIN_WIDTH, PORT_WIN_HEIGHT);

    portInputLabel = gtk_label_new("Port Number : ");
    portInputEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(portInputEntry), "Port");

    guint margin = 5;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    gtk_container_add(GTK_CONTAINER(portWin), grid);

    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(getPortText), portWin);

    gtk_grid_attach(GTK_GRID(grid), portInputLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), portInputEntry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 3, 1);

    gtk_widget_show_all(portWin);
}

void logger(char *msg) {
    GtkTextIter end;
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_get_end_iter(textBuffer, &end);

    gtk_text_buffer_insert(textBuffer, &end, msg, -1);
}

void *showMembers(void *arg) {
    static char tmp[BUF_SIZE];
    memset(tmp, 0, sizeof(tmp));
    strcpy(tmp, "[INFO] Current members : ");

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (memberInfo[i].socket == -1)
            break;
        strcat(tmp, memberInfo[i].name);

        if (i != clientCount - 1)
            strcat(tmp, ", ");
    }

    strcat(tmp, "\n");
}

static void manageWindow(GtkApplication *app, gpointer user_data) {
    // gtk_window_close((GtkWindow*)portWin);
    GtkWidget *window;
    GtkWidget *shutdownButton;
    GtkWidget *createRoomButton;
    GtkWidget *grid;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SimpleChat Server");
    gtk_window_set_default_size(GTK_WINDOW(window), MAIN_WIDTH, MAIN_HEIGHT);

    g_signal_connect(window, "delete-event", G_CALLBACK(closeRequest), NULL);

    logText = gtk_text_view_new();
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));

    guint margin = 2;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    gtk_container_add(GTK_CONTAINER(window), grid);

    shutdownButton = gtk_button_new_with_label("Shutdown");
    g_signal_connect(shutdownButton, "clicked", G_CALLBACK(closeRequest), window);

    createRoomButton = gtk_button_new_with_label("Show\nMembers");
    g_signal_connect(createRoomButton, "clicked", G_CALLBACK(showMembers), window);

    gtk_grid_attach(GTK_GRID(grid), logText, 0, 0, 5, 5);
    gtk_grid_attach(GTK_GRID(grid), createRoomButton, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), shutdownButton, 4, 5, 1, 1);

    gtk_widget_show_all(window);

    pthread_create(&serverThread, NULL, startServer, NULL);
    pthread_detach(serverThread);
}

void *handleClient(void *arg) {
    int socket = *((int *)arg);
    int length = 0;
    char msg[BUF_SIZE];

    while ((length = read(socket, msg, sizeof(msg))) != 0) {
        msg[length] = '\0';

        int tmp = 0, i;
        char prefix[NAME_SIZE], name[NAME_SIZE], data[BUF_SIZE];

        for (i = 0; msg[tmp] != ','; i++)
            prefix[i] = msg[tmp++];
        prefix[++i] = '\0';

        tmp++;
        for (i = 0; msg[tmp] != ','; i++)
            name[i] = msg[tmp++];
        name[i] = '\0';

        tmp++;
        for (i = 0; msg[tmp] != '\0'; i++)
            data[i] = msg[tmp++];

        data[i] = '\0';

        memset(buf, 0, sizeof(buf));
        if (strcmp(prefix, "new") == 0) {
            strcpy(buf, "[INFO] New member ");
            strcat(buf, name);
            strcat(buf, " connected!\n");

            logger(buf);
        } else if (strcmp(prefix, "exit") == 0) {
            strcpy(buf, "[INFO] Member ");
            strcat(buf, name);
            strcat(buf, " disconnected.\n");

            logger(buf);
        } else if (strcmp(prefix, "global") == 0) {
            buf[0] = '[';
            strcat(buf, name);
            strcat(buf, "] ");
            strcat(buf, data);
            strcat(buf, "\n");
            sendMsg(buf, strlen(buf));
            logger(buf);
        }
    }

    pthread_mutex_lock(&clientMutex);
    for (int i = 0; i < clientCount; i++) {
        if (socket == clientSockets[i]) {
            while (i < clientCount - 1) {
                clientSockets[i] = clientSockets[i + 1];
                i++;
            }

            break;
        }
    }
    clientCount--;
    pthread_mutex_unlock(&clientMutex);
    close(socket);
    return NULL;
}

void sendMsg(char *msg, int len) {
    pthread_mutex_lock(&clientMutex);
    for (int i = 0; i < clientCount; i++)
        write(clientSockets[i], msg, len);
    pthread_mutex_unlock(&clientMutex);
}
