// gcc `pkg-config --cflags gtk4` -o server server.c `pkg-config --libs gtk4`

#include <stdio.h>
#include <gtk/gtk.h>

#define MAIN_WIDTH 600
#define MAIN_HEIGHT 600

static void print_hello(GtkWidget *, gpointer);
static void activate(GtkApplication*, gpointer);

int main(int argc, char** argv) {
    GtkApplication *mainWindow;

    mainWindow = gtk_application_new("com.toygoon.simplechat", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(mainWindow, "activate", G_CALLBACK(activate), NULL);
    g_application_run(G_APPLICATION(mainWindow), argc, argv);
    g_object_unref(mainWindow);

    return 0;
}

static void print_hello(GtkWidget *widget, gpointer data) {
    g_print("Hello World\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), MAIN_WIDTH, MAIN_HEIGHT);

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), box);

    button = gtk_button_new_with_label("Hello World");
    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy), window);
    gtk_box_append(GTK_BOX(box), button);

    gtk_widget_show(window);
}