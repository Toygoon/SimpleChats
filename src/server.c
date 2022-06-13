/* Server for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include "server.h"

int main(int argc, char **argv) {
    app = gtk_application_new("yu.server.simplechat", G_APPLICATION_FLAGS_NONE);

    pthread_mutex_init(&loggerMutex, NULL);
    g_signal_connect(app, "activate", G_CALLBACK(portWindow), NULL);
    // g_signal_connect(app, "activate", G_CALLBACK(manageWindow), NULL);
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return 0;
}

gboolean closeRequest(GtkWindow *window, gpointer user_data) {
    close(serverSocket);
    gtk_window_close(window);

    return false;
}

void *startServer(void *arg) {
    pthread_mutex_init(&clientMutex, NULL);
    pthread_mutex_init(&memberMutex, NULL);
    logger("[INFO] Initiating server socket.\n");
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    logger("[INFO] Generating socket address struct.\n");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    char tmp[BUF_SIZE];
    sprintf(tmp, "[INFO] Setting a port number to %d\n", portNum);
    logger(tmp);
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

    while (true) {
        clientAddrSz = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSz);

        pthread_mutex_lock(&clientMutex);
        clientSockets[clientCount] = clientSocket;
        pthread_mutex_unlock(&clientMutex);

        pthread_create(&clientsThread, NULL, handleClient, (void *)&clientSocket);
        pthread_detach(clientsThread);
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
    gtk_entry_set_text(GTK_ENTRY(portInputEntry), "7777");

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
    pthread_mutex_lock(&loggerMutex);
    
    GtkTextIter end;
    gchar *unicode;

    if ((unicode = gtkui_utf8_validate(msg)) == NULL)
        return;

    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_get_end_iter(textBuffer, &end);
    gtk_text_buffer_insert(textBuffer, &end, unicode, -1);
    
    pthread_mutex_unlock(&loggerMutex);
}

void showMembers(void) {
    if (memberLinkedList == NULL) {
        logger("[INFO] No members connected.");

        return;
    }

    logger("[INFO] Current members : ");

    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next) {
        if (ptr->isDisabled)
            continue;

        logger(ptr->name);

        if (ptr->next != NULL)
            logger(", ");
    }
    logger("\n");
}

void newMember(int socket, char *name) {
    MemberInfo *member = (MemberInfo *)malloc(sizeof(MemberInfo));
    member->socket = socket;
    member->isDisabled = false;
    member->enteredRoom = false;
    strcpy(member->name, name);

    if (memberLinkedList == NULL)
        member->next = NULL;
    else
        member->next = memberLinkedList;

    memberLinkedList = member;
}

void removeMember(int socket) {
    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next) {
        if (ptr->isDisabled)
            continue;

        if (ptr->socket == socket)
            ptr->isDisabled = true;
    }
}

int findSocketByName(char *name) {
    int res = -1;

    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next)
        if (strcmp(name, ptr->name) == 0) {
            res = ptr->socket;

            break;
        }

    return res;
}

MemberInfo *findMemberBySocket(int socket) {
    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next)
        if (socket == ptr->socket) {
            return ptr;
        }

    return NULL;
}

int createNewRoom(int socket, char *name) {
    char tmp[BUF_SIZE];
    RoomInfo *room = (RoomInfo *)malloc(sizeof(RoomInfo));
    MemberInfo *target = findMemberBySocket(socket);
    SocketInfo *socketInfo = (SocketInfo *)malloc(sizeof(SocketInfo));

    strcpy(room->name, name);
    room->next = roomLinkedList;
    roomLinkedList = room;
    socketInfo->isDisabled = false;
    socketInfo->socket = socket;
    socketInfo->next = NULL;
    room->roomSocketList = socketInfo;

    sprintf(tmp, "enterroom,%d,%s", 1, room->name);
    g_print("%s\n", tmp);
    target->enteredRoom = true;
    write(socket, tmp, strlen(tmp));

    sprintf(tmp, "[ROOM] Created new room %s\n", room->name);
    logger(tmp);
}

void sendRoomList(int socket) {
    char tmp[BUF_SIZE];
    strcpy(tmp, "roominfo,");
    for (RoomInfo *ptr = roomLinkedList; ptr != NULL; ptr = ptr->next) {
        strcat(tmp, ptr->name);

        if (ptr->next != NULL)
            strcat(tmp, ",");
    }

    g_print("%s\n", tmp);

    write(socket, tmp, strlen(tmp));
}

void enterRoomRequest(int socket, char *roomName) {
    char tmp[BUF_SIZE];
    RoomInfo *ptr = roomLinkedList;

    for (; ptr != NULL; ptr = ptr->next) {
        if (strcmp(roomName, ptr->name) == 0)
            break;

        if (ptr->next == NULL)
            return;
    }

    SocketInfo *socketInfo = (SocketInfo *)malloc(sizeof(SocketInfo)), *bak;

    socketInfo->socket = socket;
    socketInfo->isDisabled = false;

    bak = ptr->roomSocketList;
    socketInfo->next = bak;
    ptr->roomSocketList = socketInfo;

    bak = roomLinkedList;
}

static void manageWindow(GtkApplication *app, gpointer user_data) {
    gtk_window_close((GtkWindow *)portWin);
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
    char msg[BUF_SIZE], prefix[NAME_SIZE], name[NAME_SIZE], data[BUF_SIZE];

    while ((length = read(socket, msg, sizeof(msg))) != 0) {
        memset(prefix, 0, sizeof(prefix));
        memset(name, 0, sizeof(name));
        memset(data, 0, sizeof(data));
        msg[length] = '\0';
        g_print("%s\n", msg);

        int tmp = 0, i;

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
            sprintf(buf, "[INFO] New member %s connected!\n", name);

            pthread_mutex_lock(&memberMutex);
            newMember(socket, name);
            pthread_mutex_unlock(&memberMutex);

            logger(buf);
        } else if (strcmp(prefix, "exit") == 0) {
            break;
        } else if (strcmp(prefix, "newroom") == 0) {
            createNewRoom(socket, data);
        } else if (strcmp(prefix, "room") == 0) {
            // sendRoomMsg(socket, data);
            sprintf(buf, "[%s] %s\n", name, data);
            sendGlobalMsg(buf, strlen(buf));

            logger(buf);
        } else if (strcmp(prefix, "enterroom") == 0) {
            enterRoomRequest(socket, data);
        } else if (strcmp(prefix, "roominfo") == 0) {
            sendRoomList(socket);
        } else if (strcmp(prefix, "private") == 0) {
            pthread_mutex_lock(&memberMutex);
            pthread_mutex_lock(&clientMutex);

            int destSocket;
            char private[BUF_SIZE], dest[NAME_SIZE];
            memset(private, 0, sizeof(private));
            memset(dest, 0, sizeof(dest));

            tmp = 1;
            for (i = 0; data[tmp] != ' '; i++)
                dest[i] = data[tmp++];
            dest[i] = '\0';

            tmp++;

            sprintf(buf, "(Private) [%s] ", name);
            for (i = strlen(buf); data[tmp] != '\0'; i++)
                buf[i] = data[tmp++];
            buf[i] = '\0';
            strcat(buf, "\n");

            destSocket = findSocketByName(dest);

            if (destSocket == -1) {
                sprintf(private, "[ERROR] No member named %s\n", dest);
                write(socket, private, strlen(private));

                sprintf(buf, "[ERROR] Private message %s to %s failed (invalid member name)\n", name, dest);
                logger(buf);
            } else {
                write(destSocket, buf, strlen(buf));

                sprintf(buf, "[PRIVATE] Sending private message %s to %s succeeded.\n", name, dest);
                logger(buf);
            }
            pthread_mutex_unlock(&clientMutex);
            pthread_mutex_unlock(&memberMutex);
        } else if (strcmp(prefix, "global") == 0) {
            sprintf(buf, "[%s] %s\n", name, data);
            sendGlobalMsg(buf, strlen(buf));

            logger(buf);
        }
    }

    strcpy(buf, "[INFO] Member ");
    strcat(buf, name);
    strcat(buf, " disconnected.\n");

    logger(buf);

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

    pthread_mutex_lock(&memberMutex);
    removeMember(socket);
    pthread_mutex_unlock(&memberMutex);

    close(socket);
    return NULL;
}

void sendGlobalMsg(char *msg, int len) {
    pthread_mutex_lock(&clientMutex);
    for (int i = 0; i < clientCount; i++) {
        /*
        MemberInfo *member = findMemberBySocket(clientSockets[i]);
        if (member->enteredRoom)
            continue;
        */
        write(clientSockets[i], msg, len);
    }
    pthread_mutex_unlock(&clientMutex);
}

void sendRoomMsg(int socket, char *msg) {
    bool found = false;
    RoomInfo *ptr;

    for (ptr = roomLinkedList; ptr != NULL; ptr = ptr->next) {
        for (SocketInfo *s = ptr->roomSocketList; s != NULL; s = s->next)
            if (s->socket == socket)
                found = true;

        if (found)
            break;
    }

    if (found == false)
        return;

    for (SocketInfo *s = ptr->roomSocketList; s != NULL; s = s->next) {
        if (s->socket != socket && s->isDisabled == false) {
            write(s->socket, msg, strlen(msg));
        }
    }
}

char *gtkui_utf8_validate(char *data) {
    const gchar *end;
    char *unicode = NULL;

    unicode = data;
    if (!g_utf8_validate(data, -1, &end)) {
        /* if "end" pointer is at beginning of string, we have no valid text to print */
        if (end == unicode) return (NULL);

        /* cut off the invalid part so we don't lose the whole string */
        /* this shouldn't happen often */
        unicode = (char *)end;
        *unicode = 0;
        unicode = data;
    }

    return (unicode);
}