/* Server for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include <gtk/gtk.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAIN_WIDTH 600
#define MAIN_HEIGHT 600

#define PORT_WIN_WIDTH 200
#define PORT_WIN_HEIGHT 100

#define BUF_SIZE 100
#define MAX_CLIENT 256

static void manageWindow(GtkApplication *, gpointer);
static void portWindow(GtkApplication *, gpointer);
gint deleteEvent(GtkWidget *, GdkEvent *, gpointer);
static void getPortText(GtkApplication *, gpointer);
void *handleClient(void *);
void sendMsg(char *, int);
void handleError(char *);
void *startServer(void *);
void logger(char *);

int clientCount = 0, totalChannels = 0;
int clientSockets[MAX_CLIENT];
pthread_mutex_t mutx;

char buf[BUF_SIZE];
int serverSocket, clientSocket;
struct sockaddr_in servAddr, clientAddr;
int clientAddrSz, portNum = 7777;
pthread_t threadId, serverThread;

GtkApplication *app;
GtkWidget *portWin;
GtkWidget *logText;
GtkTextBuffer *textBuffer;
GtkWidget *portInputEntry;

int main(int argc, char **argv) {
    app = gtk_application_new("com.toygoon.simplechat", G_APPLICATION_FLAGS_NONE);

    //g_signal_connect(app, "activate", G_CALLBACK(portWindow), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(manageWindow), NULL);
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return 0;
}

gboolean closeRequest(GtkWindow* window, gpointer user_data) {
    close(serverSocket);
    gtk_window_destroy(window);

    return false;
}

void *startServer(void *arg) {
    pthread_mutex_init(&mutx, NULL);
    logger("[INFO] Initiating server socket.\n");
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    logger("[INFO] Generating socket address struct.\n");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    logger("[INFO] Setting a port number.\n");
    servAddr.sin_port = htons(portNum);

    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
        logger("[ERROR] bind error, choose another port.\n");
        return false;
    }
    if (listen(serverSocket, 5) == -1) {
        logger("[ERROR] listen error, please free up your memory.\n");
        return false;
    }

    logger("[INFO] Server has started.\n");

    while (1) {
        clientAddrSz = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSz);

        pthread_mutex_lock(&mutx);
        clientSockets[clientCount++] = clientSocket;
        pthread_mutex_unlock(&mutx);

        pthread_create(&threadId, NULL, handleClient, (void *)&clientSocket);
        pthread_detach(threadId);
        char tmp[BUF_SIZE] = "[CLIENT] Connected client IP: ";
        strcat(tmp, inet_ntoa(clientAddr.sin_addr));
        strcat(tmp, "\n");
        logger(tmp);
    }
}

static void getPortText(GtkApplication *_app , gpointer user_data) {
    GtkEntryBuffer* entryBuffer = gtk_entry_get_buffer((GtkEntry*)portInputEntry);
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
    gtk_entry_set_placeholder_text(GTK_ENTRY(portInputEntry),"Port");

    guint margin = 5;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), true);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), true);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    gtk_window_set_child(GTK_WINDOW(portWin), grid);

    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(getPortText), portWin);

    gtk_grid_attach(GTK_GRID(grid), portInputLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), portInputEntry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 3, 1);

    gtk_widget_show(portWin);
}

void logger(char* msg) {
    GtkTextIter end;
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_get_end_iter (textBuffer, &end);

    gtk_text_buffer_insert (textBuffer, &end, msg, -1);
}

static void manageWindow(GtkApplication *app, gpointer user_data) {
    // gtk_window_destroy((GtkWindow*)portWin);
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *grid;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SimpleChat Server");
    gtk_window_set_default_size(GTK_WINDOW(window), MAIN_WIDTH, MAIN_HEIGHT);

    g_signal_connect(window, "close-request", G_CALLBACK(closeRequest), NULL);

    logText = gtk_text_view_new();
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));

    guint margin = 2;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), true);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), true);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    gtk_window_set_child(GTK_WINDOW(window), grid);

    button = gtk_button_new_with_label("a");
    gtk_widget_set_size_request(button, 10, 10);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(logger), NULL);

    gtk_grid_attach(GTK_GRID(grid), logText, 0, 0, 5, 5);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 1, 1);

    gtk_widget_show(window);

    pthread_create(&serverThread, NULL, startServer, NULL);
    pthread_detach(serverThread);
}

void *handleClient(void *arg) {
    int clientSocket = *((int *)arg);
    int length = 0, i;
    char msg[BUF_SIZE];

    while ((length = read(clientSocket, msg, sizeof(msg))) != 0)
        sendMsg(msg, length);

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clientCount; i++) {
        if (clientSocket == clientSockets[i]) {
            while (i < clientCount - 1) {
                clientSockets[i] = clientSockets[i + 1];
                i++;
            }

            break;
        }
    }
    clientCount--;
    pthread_mutex_unlock(&mutx);
    close(clientSocket);
    return NULL;
}

void sendMsg(char *msg, int len) {
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clientCount; i++)
        write(clientSockets[i], msg, len);
    pthread_mutex_unlock(&mutx);
}