/* Client for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 100
#define MAX_CLIENT 256

int clientCount = 0;
int clientSockets[MAX_CLIENT];
pthread_mutex_t mutx;

void *handleClient(void *);
void sendMsg(char *, int);
void handleError(char *);

int main(int argc, char *argv[]) {
    int serverSocket, clientSocket;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSocket, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        handleError("bind() error");
    if (listen(serverSocket, 5) == -1)
        handleError("listen() error");

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clientSockets[clientCount++] = clientSocket;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handleClient, (void *)&clientSocket);
        pthread_detach(t_id);
        printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serverSocket);
    return 0;
}

void *handleClient(void *arg) {
    int clientSocket = *((int *)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];

    while ((str_len = read(clientSocket, msg, sizeof(msg))) != 0)
        sendMsg(msg, str_len);

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clientCount; i++)  // remove disconnected client
    {
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