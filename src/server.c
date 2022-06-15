/* Server for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include "server.h"

int main(int argc, char **argv) {
    // 서버로 사용될 Window의 Artifact를 지정
    app = gtk_application_new("yu.server.simplechat", G_APPLICATION_FLAGS_NONE);

    // Logger Window를 위한 Mutex 초기화
    pthread_mutex_init(&loggerMutex, NULL);
    // Port를 입력받는 Window를 먼저 시작
    g_signal_connect(app, "activate", G_CALLBACK(portWindow), NULL);
    // Window 시작
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return 0;
}

/* closeRequest() : Shutdown 버튼, 혹은 창을 닫는 버튼을 눌렀을 때 호출됨 */
gboolean closeRequest(GtkWindow *window, gpointer user_data) {
    // 서버 소켓 닫음
    close(serverSocket);
    gtk_window_close(window);

    return false;
}

/* startServer() : 서버 시작, Client로부터 요청을 받음 */
void *startServer(void *arg) {
    // Client 소켓을 저장하는 Mutex 초기화
    pthread_mutex_init(&clientMutex, NULL);
    // Member 정보를 저장하는 Mutex 초기화
    pthread_mutex_init(&memberMutex, NULL);

    logger("[INFO] Initiating server socket.\n");
    // 서버 소켓 만들기
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    logger("[INFO] Generating socket address struct.\n");
    // sockaddr_in 구조체를 0으로 채움 (sockaddr로 구조체로 캐스팅하기 위함,
    // 이는 각 구조체가 차지하는 메모리의 크기가 다르기 때문에, 0으로 채워줌)
    memset(&servAddr, 0, sizeof(servAddr));
    // AF_INET : IPv4 통신을 사용하겠다는 의미
    servAddr.sin_family = AF_INET;
    // INADDR_ANY : 호스트는 루프백으로 오는 모든 요청을 받아들이겠다는 의미
    // htonl() : host order의 방식을 network byte order 방식으로 바꿔줌
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    char tmp[BUF_SIZE];
    sprintf(tmp, "[INFO] Setting a port number to %d\n", portNum);
    logger(tmp);
    // 포트 번호 설정, htons()를 통해 포트 번호를 network byte order 방식으로 변경
    servAddr.sin_port = htons(portNum);

    // bind()를 통한 포트 번호 할당이 실패했을 경우
    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
        logger("[ERROR] bind error, choose another port.\n");
        return false;
    }

    // listen()을 통한 Queue 할당이 실패했을 경우
    if (listen(serverSocket, 5) == -1) {
        logger("[ERROR] listen error, please free up your memory.\n");
        return false;
    }

    logger("[INFO] Server has started.\n");

    // 무한 루프를 이용하여 클라이언트의 요청을 받음
    while (true) {
        clientAddrSz = sizeof(clientAddr);
        // accept()를 이용하여 클라이언트의 요청을 받도록 Blocking 함수를 호출
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSz);

        // 스레드간 클라이언트 소켓을 저장하는 부분에 손실을 방지하기 위해 Mutex 이용
        pthread_mutex_lock(&clientMutex);
        // 현재 클라이언트 소켓을 저장
        clientSockets[clientCount] = clientSocket;
        // Mutex로 잠금했던 부분을 해제
        pthread_mutex_unlock(&clientMutex);

        // 클라이언트 소켓을 잘 받았으므로, 해당 클라이언트에 대한 스레드 시작
        pthread_create(&clientsThread, NULL, handleClient, (void *)&clientSocket);
        // pthread_detach()를 이용하여 서버 종료시 자원도 반납하도록 함
        pthread_detach(clientsThread);
        // 다음 클라이언트 소켓을 받아들이기 위해 clientCount 변수를 증가시킴
        clientCount++;

        char tmp[BUF_SIZE] = "[CLIENT] Connected client IP: ";
        strcat(tmp, inet_ntoa(clientAddr.sin_addr));
        strcat(tmp, "\n");
        logger(tmp);
    }
}

/* getPortText() : 포트 번호를 설정하는 Window에서 포트 번호를 읽어옴 */
static void getPortText(GtkApplication *_app, gpointer user_data) {
    // 입력 창에 사용된 버퍼를 가져옴
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)portInputEntry);
    // 해당 버퍼에서 const char* 형식의 텍스트를 읽어옴
    const char *text = gtk_entry_buffer_get_text(entryBuffer);
    // 포트 번호를 atoi(ASCII to Integer) 함수로 정수 형식으로 변환
    portNum = atoi(text);

    // 다음 Window를 띄워줌
    manageWindow(app, NULL);
}

/* getPortText() : 포트 번호를 설정하는 Window */
static void portWindow(GtkApplication *app, gpointer user_data) {
    // 버튼, 입력창, 그리드, 안내 문구 설정
    GtkWidget *button;
    GtkWidget *portInputLabel;
    GtkWidget *grid;
    gchar *text;

    // 포트 번호를 입력받는 Window의 타이틀, 너비 설정
    portWin = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(portWin), "Port");
    gtk_window_set_default_size(GTK_WINDOW(portWin), PORT_WIN_WIDTH, PORT_WIN_HEIGHT);

    // 포트 번호의 입력 안내를 위한 텍스트
    portInputLabel = gtk_label_new("Port Number : ");
    // 포트 번호를 입력받기 위한 창
    portInputEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(portInputEntry), "Port");
    gtk_entry_set_text(GTK_ENTRY(portInputEntry), "7777");

    // 각 구성 요소간의 여백 설정
    guint margin = 5;
    grid = gtk_grid_new();

    // 그리드의 상세 설정 (정렬 방식을 설정)
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    // 그리드를 윈도우에 추가
    gtk_container_add(GTK_CONTAINER(portWin), grid);

    // OK 버튼 생성
    button = gtk_button_new_with_label("OK");
    // OK 버튼을 클릭했을 때, 작동 방식
    g_signal_connect(button, "clicked", G_CALLBACK(getPortText), portWin);
    // Enter 키를 눌렀을 때도 작동하게 함
    g_signal_connect(GTK_WINDOW(portWin), "key_press_event", G_CALLBACK(buttonPressed), NULL);

    // 버튼과 입력창 등을 그리드에 추가
    gtk_grid_attach(GTK_GRID(grid), portInputLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), portInputEntry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 3, 1);

    // Window 띄우기
    gtk_widget_show_all(portWin);
}

/* logger() : msg로 전달받은 char* 형식을 logger에 추가해줌 */
void logger(char *msg) {
    // 한 번에 한 요청만 받아들일 수 있게 Mutex 사용
    pthread_mutex_lock(&loggerMutex);
    
    // Iterator 설정 (마지막 요소를 가리키도록 함)
    GtkTextIter end;
    // Unicode인지 확인
    gchar *unicode;

    if ((unicode = gtkui_utf8_validate(msg)) == NULL)
        return;

    // Logger의 버퍼를 가져옴
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_get_end_iter(textBuffer, &end);
    // 텍스트 추가
    gtk_text_buffer_insert(textBuffer, &end, unicode, -1);

    // Mutex 잠금 해제
    pthread_mutex_unlock(&loggerMutex);
}

/* showMembers() : 현재 사용자의 이름을 출력 */
void showMembers(void) {
    // 멤버가 없을 경우
    if (memberLinkedList == NULL) {
        logger("[INFO] No members connected.");

        return;
    }

    logger("[INFO] Current members : ");
    // memberLinkedList를 순회하면서 출력
    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next) {
        // 비활성화된 사용자는 건너뜀
        if (ptr->isDisabled)
            continue;

        // logger에 추가
        logger(ptr->name);

        if (ptr->next != NULL)
            logger(", ");
    }
    logger("\n");
}

/* newMember() : 새 사용자를 memberLinkedList에 추가하는 함수 */
void newMember(int socket, char *name) {
    // 새 MemberInfo 구조체 포인터의 할당
    MemberInfo *member = (MemberInfo *)malloc(sizeof(MemberInfo));
    member->socket = socket;
    member->isDisabled = false;
    member->enteredRoom = false;
    strcpy(member->name, name);

    // memberLinkedList에 추가
    if (memberLinkedList == NULL)
        member->next = NULL;
    else
        member->next = memberLinkedList;

    memberLinkedList = member;
}

/* newMember() : 새 사용자를 memberLinkedList에서 비활성화하는 함수 */
void removeMember(int socket) {
    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next) {
        if (ptr->isDisabled)
            continue;

        // 소켓을 찾으면 bool 변화
        if (ptr->socket == socket)
            ptr->isDisabled = true;
    }
}

/* findSocketByName() : 사용자의 이름을 소켓 Descriptor로 찾음 */
int findSocketByName(char *name) {
    int res = -1;

    // memberLinkedList를 순회하면서 탐색
    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next)
        if (strcmp(name, ptr->name) == 0) {
            res = ptr->socket;

            break;
        }

    // 찾은 결과를 반환
    return res;
}

/* findMemberBySocket() : 사용자의 MemberInfo 포인터를 사용자 이름으로 찾음 */
MemberInfo *findMemberBySocket(int socket) {
    // memberLinkedList를 순회하면서 탐색
    for (MemberInfo *ptr = memberLinkedList; ptr != NULL; ptr = ptr->next)
        // 찾았을 경우 Return
        if (socket == ptr->socket)
            return ptr;

    // 탐색 실패의 경우 NULL 반환
    return NULL;
}

/* createNewRoom() : 요청한 소켓에서 방 이름을 통해 방을 생성하도록 함 */
int createNewRoom(int socket, char *name) {
    char tmp[BUF_SIZE];
    // 방 정보를 나타내는 RoomInfo 포인터의 메모리 할당
    RoomInfo *room = (RoomInfo *)malloc(sizeof(RoomInfo));
    // 요청한 소켓의 멤버 정보를 찾음
    MemberInfo *target = findMemberBySocket(socket);
    // 소켓 정보를 나타내는 SocketInfo 포인터의 메모리 할당, 채팅방 멤버에게만 전송하기 위한 용도
    SocketInfo *socketInfo = (SocketInfo *)malloc(sizeof(SocketInfo));

    strcpy(room->name, name);
    // roomLinkedList에 추가
    room->next = roomLinkedList;
    roomLinkedList = room;
    socketInfo->isDisabled = false;
    socketInfo->socket = socket;
    socketInfo->next = NULL;
    room->roomSocketList = socketInfo;

    // Prefix는 enterroom 형식을 이용
    sprintf(tmp, "enterroom,%d,%s", 1, room->name);
    g_print("%s\n", tmp);
    target->enteredRoom = true;
    write(socket, tmp, strlen(tmp));

    sprintf(tmp, "[ROOM] Created new room %s\n", room->name);
    logger(tmp);
}

/* sendRoomList() : 방 이름을 모두 출력하도록 함 */
void sendRoomList(int socket) {
    char tmp[BUF_SIZE];
    strcpy(tmp, "roominfo,");

    // roomLinkedList를 순회하며 출력
    for (RoomInfo *ptr = roomLinkedList; ptr != NULL; ptr = ptr->next) {
        strcat(tmp, ptr->name);

        if (ptr->next != NULL)
            strcat(tmp, ",");
    }

    g_print("%s\n", tmp);
    // 요청한 socket에게 전달
    write(socket, tmp, strlen(tmp));
}

/** sendRoomList() : 해당 socket을 roomLinkedList의 socketInfo에 추가하여
 * 해당 socket을 가진 클라이언트에게만 메시지를 전송하도록 함 */
void enterRoomRequest(int socket, char *roomName) {
    char tmp[BUF_SIZE];
    RoomInfo *ptr = roomLinkedList;

    // 방 정보 탐색
    for (; ptr != NULL; ptr = ptr->next) {
        if (strcmp(roomName, ptr->name) == 0)
            break;

        if (ptr->next == NULL)
            return;
    }

    // 새로운 클라이언트 소켓을 추가
    SocketInfo *socketInfo = (SocketInfo *)malloc(sizeof(SocketInfo)), *bak;

    // 아래는 LinkedList의 추가 방식과 동일
    socketInfo->socket = socket;
    socketInfo->isDisabled = false;

    bak = ptr->roomSocketList;
    socketInfo->next = bak;
    ptr->roomSocketList = socketInfo;

    bak = roomLinkedList;
}

/* manageWindow() : 서버를 관리하는 Window */
static void manageWindow(GtkApplication *app, gpointer user_data) {
    // 포트 번호를 입력받는 창을 닫음
    gtk_window_close((GtkWindow *)portWin);
    // Window를 구성하는 버튼, 그리드
    GtkWidget *window;
    GtkWidget *shutdownButton;
    GtkWidget *createRoomButton;
    GtkWidget *grid;

    // 새 Window 생성
    window = gtk_application_window_new(app);
    // 타이틀, 너비 설정
    gtk_window_set_title(GTK_WINDOW(window), "SimpleChat Server");
    gtk_window_set_default_size(GTK_WINDOW(window), MAIN_WIDTH, MAIN_HEIGHT);

    // 창을 닫았을 때 호출할 함수를 설정 (소켓을 닫기 위함)
    g_signal_connect(window, "delete-event", G_CALLBACK(closeRequest), NULL);

    // Log를 보여주는 텍스트 부분 설정
    logText = gtk_text_view_new();
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));

    // 그리드에서의 요소 간의 여백 설정
    guint margin = 2;
    grid = gtk_grid_new();

    // 그리드 상세 설정
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    // 그리드 추가
    gtk_container_add(GTK_CONTAINER(window), grid);

    // 서버 종료 버튼 설정
    shutdownButton = gtk_button_new_with_label("Shutdown");
    g_signal_connect(shutdownButton, "clicked", G_CALLBACK(closeRequest), window);

    // 현재 접속 중인 멤버를 출력, showMembers 함수 호출
    createRoomButton = gtk_button_new_with_label("Show\nMembers");
    g_signal_connect(createRoomButton, "clicked", G_CALLBACK(showMembers), window);

    // 각 구성 요소를 추가
    gtk_grid_attach(GTK_GRID(grid), logText, 0, 0, 5, 5);
    gtk_grid_attach(GTK_GRID(grid), createRoomButton, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), shutdownButton, 4, 5, 1, 1);

    // Window를 띄움
    gtk_widget_show_all(window);

    // startServer를 스레드로 시작
    pthread_create(&serverThread, NULL, startServer, NULL);
    // detach를 호출하여 자원 반환, Blcoking 방지
    pthread_detach(serverThread);
}

/* handleClient() : 새로운 클라이언트가 접속했을 경우 스레드로 시작 */
void *handleClient(void *arg) {
    // 전달 받은 소켓 정보
    int socket = *((int *)arg);
    // read 함수에서 받아올 strlen
    int length = 0;
    char msg[BUF_SIZE], prefix[NAME_SIZE], name[NAME_SIZE], data[BUF_SIZE];

    // 무한 루프를 통해 클라이언트의 모든 요청을 받음
    while ((length = read(socket, msg, sizeof(msg))) != 0) {
        memset(prefix, 0, sizeof(prefix));
        memset(name, 0, sizeof(name));
        memset(data, 0, sizeof(data));
        msg[length] = '\0';
        g_print("%s\n", msg);

        int tmp = 0, i;

        /** 클라이언트가 전송하는 메시지의 형식
         * global,Name,MSG : Name이 모든 클라이언트에게 전송하는 MSG
         * private,Name2,@Name3,MSG : Name2가 Name3에게 전송하는 MSG */

        /* strtok을 이용할 경우, 잘라낸 위치만을 가지는 포인터만 반환되기 때문에,
        해당 str를 다른 변수로 이용할 수 없음, 또한 read로 전달받은 데이터는
        length가 strlen을 통해 구성할 경우 다른 크기가 오게 되어 strcpy 또한 이용하기
        힘든 상황이 발생하였으므로, for 반복문을 통해 직접 문자열을 잘라냄 */

        // Prefix, 클라이언트가 보내는 요청의 형식을 잘라냄
        for (i = 0; msg[tmp] != ','; i++)
            prefix[i] = msg[tmp++];
        prefix[++i] = '\0';

        // 클라이언트의 이름을 잘라냄
        tmp++;
        for (i = 0; msg[tmp] != ','; i++)
            name[i] = msg[tmp++];
        name[i] = '\0';

        // 메시지를 잘라냄
        tmp++;
        for (i = 0; msg[tmp] != '\0'; i++)
            data[i] = msg[tmp++];
        data[i] = '\0';

        memset(buf, 0, sizeof(buf));
        if (strcmp(prefix, "new") == 0) {
            // 새 멤버가 접속했을 경우
            sprintf(buf, "[INFO] New member %s connected!\n", name);

            // 멤버의 이름을 등록
            // 멤버를 저장하는 곳은 Mutex를 이용하여 추가될 동안 잠금
            pthread_mutex_lock(&memberMutex);
            newMember(socket, name);
            pthread_mutex_unlock(&memberMutex);

            logger(buf);
        } else if (strcmp(prefix, "exit") == 0) {
            // 클라이언트가 종료했을 경우
            break;
        } else if (strcmp(prefix, "newroom") == 0) {
            // 클라이언트가 새 방을 만들고자 할 경우
            createNewRoom(socket, data);
        } else if (strcmp(prefix, "room") == 0) {
            // 채팅방에서 메시지를 보낸 경우
            // sendRoomMsg(socket, data);
            sprintf(buf, "[%s] %s\n", name, data);
            sendGlobalMsg(buf, strlen(buf));

            logger(buf);
        } else if (strcmp(prefix, "enterroom") == 0) {
            // 클라이언트가 방에 접속하고자 할 경우
            enterRoomRequest(socket, data);
        } else if (strcmp(prefix, "roominfo") == 0) {
            // 클라이언트에게 방 이름을 모두 보내줘야 할 경우
            sendRoomList(socket);
        } else if (strcmp(prefix, "private") == 0) {
            // 개인 1:1 메시지
            // 개인 메시지를 보내는 동안 멤버에 변화가 생기면 안 되므로, Mutex 이용
            pthread_mutex_lock(&memberMutex);
            pthread_mutex_lock(&clientMutex);

            // 메시지를 받을 socket을 찾음
            int destSocket;
            char private[BUF_SIZE], dest[NAME_SIZE];
            memset(private, 0, sizeof(private));
            memset(dest, 0, sizeof(dest));

            // 내용을 잘라냄
            tmp = 1;
            for (i = 0; data[tmp] != ' '; i++)
                dest[i] = data[tmp++];
            dest[i] = '\0';
            tmp++;

            // buf에 해당 형식을 저장
            sprintf(buf, "(Private) [%s] ", name);
            for (i = strlen(buf); data[tmp] != '\0'; i++)
                buf[i] = data[tmp++];
            buf[i] = '\0';
            strcat(buf, "\n");

            // 소켓 번호를 찾음
            destSocket = findSocketByName(dest);

            if (destSocket == -1) {
                // 멤버 검색 실패
                sprintf(private, "[ERROR] No member named %s\n", dest);
                write(socket, private, strlen(private));

                sprintf(buf, "[ERROR] Private message %s to %s failed (invalid member name)\n", name, dest);
                logger(buf);
            } else {
                // 멤버 검색 성공, 메시지 전송
                write(destSocket, buf, strlen(buf));

                sprintf(buf, "[PRIVATE] Sending private message %s to %s succeeded.\n", name, dest);
                logger(buf);
            }

            // Mutex 잠금 해제
            pthread_mutex_unlock(&clientMutex);
            pthread_mutex_unlock(&memberMutex);
        } else if (strcmp(prefix, "global") == 0) {
            // 모든 클라이언트에게 메시지를 보냄
            sprintf(buf, "[%s] %s\n", name, data);
            sendGlobalMsg(buf, strlen(buf));

            logger(buf);
        }
    }

    // Exit가 들어올 경우, 멤버가 종료된 경우임
    strcpy(buf, "[INFO] Member ");
    strcat(buf, name);
    strcat(buf, " disconnected.\n");
    logger(buf);

    // Client 소켓을 삭제
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

    // memberLinkedList에서도 삭제
    pthread_mutex_lock(&memberMutex);
    removeMember(socket);
    pthread_mutex_unlock(&memberMutex);

    close(socket);
    return NULL;
}

/* sendGlobalMsg() : 모든 멤버에게 메시지 전송 */
void sendGlobalMsg(char *msg, int len) {
    // 전송하는 동안 클라이언트 소켓을 담는 곳에 변화가 생기면 안 되므로, Mutex 이용
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

/* sendRoomMsg() : 채팅방에 있는 멤버에게만 메시지 전송 */
void sendRoomMsg(int socket, char *msg) {
    // 방을 찾은 경우
    bool found = false;
    RoomInfo *ptr;
    
    // 방을 찾음
    for (ptr = roomLinkedList; ptr != NULL; ptr = ptr->next) {
        for (SocketInfo *s = ptr->roomSocketList; s != NULL; s = s->next)
            if (s->socket == socket)
                found = true;

        // 찾았으면 반복문 종료
        if (found)
            break;
    }

    // 방을 찾지 못했을 경우
    if (found == false)
        return;

    // 해당 방에 존재하는 모든 소켓에게 전송
    for (SocketInfo *s = ptr->roomSocketList; s != NULL; s = s->next)
        if (s->socket != socket && s->isDisabled == false)
            write(s->socket, msg, strlen(msg));
}

/* gtkui_utf8_validate() : utf8 형식을 사용하는지 검사 */
char *gtkui_utf8_validate(char *data) {
    const gchar *end;
    char *unicode = NULL;

    unicode = data;
    if (!g_utf8_validate(data, -1, &end)) {
        // 텍스트가 없음
        if (end == unicode)
            return (NULL);

        // 메시지의 손실을 방지
        unicode = (char *)end;
        *unicode = 0;
        unicode = data;
    }

    return (unicode);
}

/* buttonPressed() : 키보드 입력의 처리 */
static gboolean buttonPressed(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Return) {
        // 엔터키 입력의 경우
        if (strcmp(gtk_window_get_title(widget), "Port") == 0) {
            // OK 버튼을 누른 것과 같은 상황을 만들어 줌
            getPortText((GtkWindow *)portWin, NULL);
            return true;
        }
    }

    return false;
}