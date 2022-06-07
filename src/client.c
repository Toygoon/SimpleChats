/* CLient for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include "client.h"

int main(int argc, char **argv) {
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);

    // g_signal_connect(app, "activate", G_CALLBACK(loginWindow), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(mainWindow), NULL);
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return 0;
}

static void getLoginData(GtkApplication *_app, gpointer user_data) {
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputEntries[0]);
    // serverIP = gtk_entry_buffer_get_text(entryBuffer);

    g_print("ip : %s\n", serverIP);

    entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputEntries[1]);
    const char *text = gtk_entry_buffer_get_text(entryBuffer);
    portNum = atoi(text);

    g_print("port : %d\n", portNum);

    entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputEntries[2]);
    // clientName = gtk_entry_buffer_get_text(entryBuffer);
    g_print("name : %s\n", clientName);

    mainWindow(app, NULL);
}

void *receiveData(void *args) {
    char buffer[NAME_SIZE + BUF_SIZE];
    int res;

    while (1) {
        res = read(clientSocket, buffer, NAME_SIZE + BUF_SIZE - 1);
        if (res == -1)
            return (void *)-1;

        buffer[res] = '\0';

        if (buffer[0] != '[') {
            char prefix[NAME_SIZE], tmp[NAME_SIZE];

            int i;
            for (i = 0; buffer[i] != ','; i++)
                prefix[i] = buffer[i];
            prefix[i++] = '\0';

            if (strcmp(prefix, "newroom") == 0) {
                roomNum = atoi(buffer[i]);
                g_print("roomNum : %d\n", roomNum);
            }
        }
        GtkTextIter end;
        logTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
        gtk_text_buffer_get_end_iter(logTextBuffer, &end);
        gtk_text_buffer_insert(logTextBuffer, &end, (const gchar *)buffer, -1);
    }

    return NULL;
}

char *createMsg(char *prefix, char *userName, char *data) {
    static char buffer[BUF_SIZE];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, prefix);
    strcat(buffer, ",");
    strcat(buffer, clientName);
    strcat(buffer, ",");
    strcat(buffer, data);

    return &buffer;
}

void *connectServer(void *args) {
    void *threadReturn;
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(serverIP);
    servAddr.sin_port = htons(portNum);

    logger("[INFO] Connecting to the server...\n");

    if (connect(clientSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
        g_print("connection error");
        exit(1);
    }

    char *data = createMsg("new", clientName, "abc");
    write(clientSocket, data, strlen(data));

    logger("[INFO] Connected!\n");
}

void logger(char *msg) {
    GtkTextIter end;
    logTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_get_end_iter(logTextBuffer, &end);

    gtk_text_buffer_insert(logTextBuffer, &end, msg, -1);
}

gboolean closeRequest(GtkWindow *window, gpointer user_data) {
    char buffer[BUF_SIZE];

    char *data = createMsg("exit", clientName, "abc");
    write(clientSocket, data, strlen(data));

    close(clientSocket);
    gtk_window_close(window);

    return false;
}

void sendText(void) {
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputText);
    const char *msg = gtk_entry_buffer_get_text(entryBuffer);

    if (strlen(msg) < 1) {
        return;
    } else if (msg[0] == '@') {
        char *data = createMsg("private", clientName, msg);
        write(clientSocket, data, strlen(data));
    } else if (msg[0] == '/') {
    } else if (roomNum != -1) {
        char prefix[NAME_SIZE];
        sprintf(prefix, "%s,%d", "room", roomNum);
    } else {
        char *data = createMsg("global", clientName, msg);
        write(clientSocket, data, strlen(data));
    }
    gtk_entry_set_text((GtkEntry *)inputText, "");
}

void sendRoomRequest(GtkApplication *_app, GtkApplication *entry) {
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)entry);
    const char *msg = gtk_entry_buffer_get_text(entryBuffer);

    char *data = createMsg("newroom", clientName, msg);
    write(clientSocket, data, strlen(data));
}

void createRoomRequest(GtkApplication *_app, gpointer user_data) {
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *grid;
    GtkWidget *window;
    gchar *text;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Create new room");
    gtk_window_set_default_size(GTK_WINDOW(window), ROOM_WIN_WIDTH, ROOM_WIN_HEIGHT);

    label = gtk_label_new("Name : ");
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "New Room");

    guint margin = 5;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    gtk_container_add(GTK_CONTAINER(window), grid);

    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(sendRoomRequest), (GtkWidget *)entry);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 3, 1);

    gtk_widget_show_all(window);
}

static void mainWindow(GtkApplication *app, gpointer user_data) {
    // gtk_window_close((GtkWindow*)loginWin);
    GtkWidget *window;
    GtkWidget *sendButton;
    GtkWidget *exitButton;
    GtkWidget *grid;
    GtkWidget *newRoom;
    void *threadReturn;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SimpleChat Client");
    gtk_window_set_default_size(GTK_WINDOW(window), MAIN_WIDTH, MAIN_HEIGHT);
    g_signal_connect(window, "delete-event", G_CALLBACK(closeRequest), NULL);

    logText = gtk_text_view_new();
    logTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));

    inputText = gtk_entry_new();
    gtk_entry_grab_focus_without_selecting(GTK_ENTRY(inputText));

    guint margin = 2;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    sendButton = gtk_button_new_with_label("Send");
    g_signal_connect(sendButton, "clicked", G_CALLBACK(sendText), NULL);

    exitButton = gtk_button_new_with_label("Exit");
    g_signal_connect(exitButton, "clicked", G_CALLBACK(closeRequest), NULL);

    newRoom = gtk_button_new_with_label("New\nRoom");
    g_signal_connect(newRoom, "clicked", G_CALLBACK(createRoomRequest), NULL);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_grid_attach(GTK_GRID(grid), logText, 0, 0, 5, 5);
    gtk_grid_attach(GTK_GRID(grid), inputText, 0, 5, 5, 1);
    gtk_grid_attach(GTK_GRID(grid), exitButton, 6, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), newRoom, 6, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sendButton, 6, 5, 1, 1);
    gtk_widget_show_all(window);

    pthread_create(&connectThread, NULL, connectServer, NULL);
    pthread_join(connectThread, &threadReturn);

    pthread_create(&receiveThread, NULL, receiveData, NULL);
    pthread_detach(receiveThread);
}

static void loginWindow(GtkApplication *app, gpointer user_data) {
    GtkWidget *button;
    GtkWidget *inputLabels[3];
    GtkWidget *grid;
    gchar *text;

    loginWin = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(loginWin), "Login");
    gtk_window_set_default_size(GTK_WINDOW(loginWin), LOGIN_WIN_WIDTH, LOGIN_WIN_HEIGHT);

    inputLabels[0] = gtk_label_new("IP : ");
    inputEntries[0] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(inputEntries[0]), "127.0.0.1");

    inputLabels[1] = gtk_label_new("Port : ");
    inputEntries[1] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(inputEntries[1]), "7777");

    inputLabels[2] = gtk_label_new("Name : ");
    inputEntries[2] = gtk_entry_new();

    guint margin = 5;
    grid = gtk_grid_new();

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    gtk_container_add(GTK_CONTAINER(loginWin), grid);

    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(getLoginData), loginWin);

    gtk_grid_attach(GTK_GRID(grid), inputLabels[0], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputEntries[0], 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), inputLabels[1], 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputEntries[1], 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), inputLabels[2], 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputEntries[2], 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 3, 1);

    gtk_widget_show_all(loginWin);
}
