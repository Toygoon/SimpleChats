// gcc `pkg-config --cflags gtk4` -o server server.c `pkg-config --libs gtk4`

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

static void activateApp(GtkApplication *, gpointer);
gint deleteEvent(GtkWidget *, GdkEvent *, gpointer);

int main(int argc, char **argv) {
    GtkApplication *mainWindow;

    mainWindow = gtk_application_new("com.toygoon.simplechat", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(mainWindow, "activate", G_CALLBACK(activateApp), NULL);
    g_application_run(G_APPLICATION(mainWindow), argc, argv);
    g_object_unref(mainWindow);

    return 0;
}

gboolean closeRequest(GtkWindow* window, gpointer user_data) {
    return 0;
}

static void activateApp(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *grid;
    GtkWidget *logText;
    GtkTextBuffer *textBuffer;

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
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy), window);
    
    gtk_grid_attach(GTK_GRID(grid), logText, 0, 0, 5, 5);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 1, 1);

    gtk_widget_show(window);
}