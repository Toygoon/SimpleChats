/* Client for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 100
#define MAX_CLIENT 256

int clientCount = 0, totalChannels = 0;
int clientSockets[MAX_CLIENT];
pthread_mutex_t mutx;
struct winsize windowSize;

void *handleClient(void *);
void sendMsg(char *, int);
void handleError(char *);
void printTitle(char *);
void printMsg(char *, char *, bool);
void printServInfo();

int main(int argc, char *argv[]) {
    FILE *html;
    char buf[BUF_SIZE];
    int serverSocket, clientSocket;
    struct sockaddr_in servAddr, clientAddr;
    int clientAddrSz, portNum = 0;
    pthread_t threadId;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);

    system("clear");
    printTitle("Chat Server");

    if (argc != 2) {
        while (portNum == 0) {
            printf("[INFO] Port number needs to be specified.\n");
            printf("[INPUT] Input the port number : ");
            fgets(buf, sizeof(buf), stdin);
            portNum = atoi(buf);
            printf("\n");
        }
    } else {
        portNum = atoi(argv[1]);
    }

    printf("[INFO] The server will start with the information below.\n");

    pthread_mutex_init(&mutx, NULL);
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(portNum);

    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1)
        handleError("bind() error");
    if (listen(serverSocket, 5) == -1)
        handleError("listen() error");

    while (1) {
        clientAddrSz = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSz);

        pthread_mutex_lock(&mutx);
        clientSockets[clientCount++] = clientSocket;
        pthread_mutex_unlock(&mutx);

        pthread_create(&threadId, NULL, handleClient, (void *)&clientSocket);
        pthread_detach(threadId);
        printf("Connected client IP: %s \n", inet_ntoa(clientAddr.sin_addr));
    }

    close(serverSocket);
    return 0;
}

void printTitle(char *title) {
    int hyphens = (windowSize.ws_col - strlen(title) - 4) / 2 - 1;

    for (int i = 0; i < 2; i++) {
        if (i == 1)
            printf("[ %s ]", title);
        for (int j = 0; j < hyphens; j++)
            printf("-");
    }

    printf("\n");
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

void handleError(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}