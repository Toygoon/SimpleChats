/* CLient for SimpleChats
 *
 * @author Lim Jung Min,
 * Department of Computer Engineering, Yeungnam University.
 */

#include "client.h"

int main(int argc, char **argv) {
    // 서버로 사용될 Window의 Artifact를 지정
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);

    // Logger Window를 위한 Mutex 초기화
    pthread_mutex_init(&loggerMutex, NULL);
    // IP, Port, 이름을 입력받는 Window를 먼저 시작
    g_signal_connect(app, "activate", G_CALLBACK(loginWindow), NULL);
    // Window 시작
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return 0;
}

/* getPortText() : IP, 포트, 이름을 설정하는 Window에서 데이터를 읽어옴 */
static void getLoginData(GtkApplication *_app, gpointer user_data) {
    // IP 입력 창에 사용된 버퍼 : inputEntries[0]
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputEntries[0]);
    // serverIP에 복사
    strcpy(serverIP, gtk_entry_buffer_get_text(entryBuffer));

    // 포트 번호 입력 창에 사용된 버퍼 : inputEntries[1]
    entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputEntries[1]);
    const char *text = gtk_entry_buffer_get_text(entryBuffer);
    // 포트 번호를 atoi(ASCII to Integer) 함수로 정수 형식으로 변환
    portNum = atoi(text);

    // 이름 입력 창에 사용된 버퍼 : inputEntries[2]
    entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputEntries[2]);
    // clientName에 복사
    strcpy(clientName, gtk_entry_buffer_get_text(entryBuffer));

    // mainWindow 호출
    mainWindow(app, NULL);
}

/* receiveData() : 스레드로 서버의 write를 받아옴 */
void *receiveData(void *args) {
    char buffer[NAME_SIZE + BUF_SIZE];
    int res;

    // 무한 루프를 통해 서버의 모든 요청을 받음
    while (true) {
        /** read() 함수를 통해 서버의 값을 받아들일 수 있다.
         * 현재 클라이언트의 socket descriptor로부터 받아들인 값을
         * read 함수를 호출함으로 결과 값의 길이를 나타낸다.ㅁ
         * 만약 -1이라면, 정상적인 전달 값이 아니게 된다. */
        res = read(clientSocket, buffer, NAME_SIZE + BUF_SIZE - 1);
        if (res == -1)
            return (void *)-1;

        buffer[res] = '\0';

        if (buffer[0] != '[' && buffer[0] != '(') {
            // 괄호로 시작하지 않은 경우, 다른 요청이므로 이를 감지
            char prefix[NAME_SIZE], nametmp[NAME_SIZE];

            // 접두사를 잘라냄
            int i, tmp = 0;
            for (i = 0; buffer[tmp] != ','; i++)
                prefix[i] = buffer[tmp++];
            prefix[i] = '\0';

            if (strcmp(prefix, "roominfo") == 0) {
                // roominfo의 경우 방의 정보를 가져오는 경우
                showRoomListWindow(buffer);
                g_print("%s\n", buffer);
            } else if (strcmp(prefix, "enterroom") == 0) {
                // enterroom의 경우 방 입장에 성공한 경우
                for (i = 0; buffer[tmp] != ','; i++)
                    tmp++;
                tmp++;

                // 방 이름을 잘라냄
                for (i = 0; buffer[tmp] != '\0'; i++)
                    nametmp[i] = buffer[tmp++];
                nametmp[i] = '\0';

                sprintf(buffer, "[ROOM] Entered room : %s\n", nametmp);
                // 방에 입장했으므로, 기존의 텍스트는 삭제
                clearLogger();
                logger(buffer);
            } else if (strcmp(prefix, "room") == 0) {
                // 방에 들어온 경우 전달받은 텍스트
                logger(buffer);
            }
        } else {
            // Global 메시지를 출력
            logger(buffer);
        }
    }

    return NULL;
}

/* createMsg() : 서버로 전달해야 할 메시지를 생성해줌 */
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

/* connectServer() : 서버로 요청을 시작하고, receiveData 스레드로 요청을 넘겨줌*/
void *connectServer(void *args) {
    // 스레드가 반환한 값을 저장하기 위한 임시 포인터
    void *threadReturn;
    // 클라이언트 소켓 생성
    // PF_INET : IPv4 사용, SOCK_STREAM : TCP 이용
    // 상세 설명은 서버의 41번째 줄에 기재되어 있다.
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    // sockaddr_in 구조체를 0으로 채움 (sockaddr로 구조체로 캐스팅하기 위함,
    // 이는 각 구조체가 차지하는 메모리의 크기가 다르기 때문에, 0으로 채워줌)
    // 이 과정이 필요한 이유는, 서버의 69번째 줄에 기재되어 있다.
    memset(&servAddr, 0, sizeof(servAddr));
    // AF_INET : IPv4 통신을 사용하겠다는 의미
    servAddr.sin_family = AF_INET;
    // inet_addr을 통해 dot-decimal 형식의 IP 주소를 Network byte order의 32비트 형식으로 변환
    servAddr.sin_addr.s_addr = inet_addr(serverIP);
    // 포트 번호 설정, htons()를 통해 포트 번호를 network byte order 방식으로 변경
    servAddr.sin_port = htons(portNum);

    logger("[INFO] Connecting to the server...\n");
    /** connect() 함수를 통해 생성했던 클라이언트 socket descriptor를 전달하고,
     * 서버의 sockaddr_in 구조체를 전달받기 위해 전역변수로 선언된 servAddr의
     * 주소를 connect 함수로 넘겨주었다. 이를 토대로, 서버에 접근할 수 있게 된다.
     * 만약 -1을 반환했다면, 연결이 실패한 경우이다. */
    if (connect(clientSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
        // connect() 함수가 실패한 경우
        g_print("connection error");
        exit(1);
    }

    // 새로운 클라이언트가 접속했다는 메시지를 띄워주기 위함
    char *data = createMsg("new", clientName, "abc");
    /** write() 함수를 통해 서버에 메시지를 전송할 수 있다.
     * clientSocket은 현재 서버와 통신하고 있으므로, 해당 socket을 이용하여
     * write() 함수를 호출하면 값을 보낼 수 있게 된다. */
    write(clientSocket, data, strlen(data));

    logger("[INFO] Connected!\n");
}

/* logger() : msg로 전달받은 char* 형식을 logger에 추가해줌 */
void logger(char *msg) {
    // 한 번에 한 요청만 받아들일 수 있게 Mutex 사용
    pthread_mutex_lock(&loggerMutex);

    // Iterator 설정 (마지막 요소를 가리키도록 함)
    GtkTextIter end;
    // Unicode인지 확인
    gchar *unicode;

    strcat(msg, "\0");

    if ((unicode = gtkui_utf8_validate(msg)) == NULL)
        return;

    // Logger의 버퍼를 가져옴
    logTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_get_end_iter(logTextBuffer, &end);
    // 텍스트 추가
    gtk_text_buffer_insert(logTextBuffer, &end, unicode, -1);

    // Mutex 잠금 해제
    pthread_mutex_unlock(&loggerMutex);
}

/* clearLogger() : logger의 모든 텍스트를 비워줌 */
void clearLogger(void) {
    logTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_buffer_set_text(logTextBuffer, "", 0);
}

/* closeRequest() : Exit 버튼, 혹은 창을 닫는 버튼을 눌렀을 때 호출됨 */
gboolean closeRequest(GtkWindow *window, gpointer user_data) {
    char buffer[BUF_SIZE];

    // 서버에 exit를 보냄
    char *data = createMsg("exit", clientName, "abc");
    write(clientSocket, data, strlen(data));

    // Client 소켓 닫음
    close(clientSocket);
    gtk_window_close(window);

    return false;
}

/* sendText() : 입력 창의 텍스트를 서버에 보냄 */
void sendText(void) {
    // 입력 창의 버퍼를 가져옴
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)inputText);
    // 입력 창의 const char*를 읽어옴
    const char *msg = gtk_entry_buffer_get_text(entryBuffer);
    char *data;

    if (strlen(msg) < 1) {
        // 메시지가 없는 경우
        return;
    } else if (msg[0] == '@') {
        // @로 시작되는 경우, 개인 메시지이므로 이에 대한 메시지를 생성
        data = createMsg("private", clientName, msg);
    } else if (roomEntered) {
        // 방에 접속한 경우, 해당 방의 메시지만 읽어옴
        data = createMsg("room", clientName, msg);
    } else {
        // 모두에게 전송하는 메시지를 보냄
        data = createMsg("global", clientName, msg);
    }

    // write 함수를 통해 클라이언트 소켓을 이용한 서버 전송
    write(clientSocket, data, strlen(data));
    // 텍스트 입력 창을 비워줌
    gtk_entry_set_text((GtkEntry *)inputText, "");
    // 클릭하지 않아도 텍스트 창에 Focus가 맞춰지도록 함
    gtk_entry_grab_focus_without_selecting(GTK_ENTRY(inputText));
}

/* sendRoomRequest() : 방 입력 창의 텍스트를 서버에 보냄 */
void sendRoomRequest(GtkApplication *_app, GtkApplication *entry) {
    // 방 이름을 입력받는 버퍼
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)entry);
    const char *msg = gtk_entry_buffer_get_text(entryBuffer);

    // newroom 접두사를 붙여 서버에 전송
    char *data = createMsg("newroom", clientName, msg);
    // 클라이언트 소켓을 이용하여 내용을 전송
    write(clientSocket, data, strlen(data));
    // 방에 입장했다는 bool을 변경
    roomEntered = true;

    // 방 입력 창을 닫음
    gtk_window_close((GtkWindow *)_app);
}

/* sendEnterRoomRequest() : 방 입장 요청을 보냄 */
void sendEnterRoomRequest(GtkApplication *_app, GtkApplication *entry) {
    // 방 이름을 입력받는 버퍼
    GtkEntryBuffer *entryBuffer = gtk_entry_get_buffer((GtkEntry *)entry);
    const char *msg = gtk_entry_buffer_get_text(entryBuffer);

    // enterroom 접두사를 붙여 서버에 전송
    char *data = createMsg("enterroom", clientName, msg);
    // 클라이언트 소켓을 이용하여 내용을 전송
    write(clientSocket, data, strlen(data));
    // 방에 입장했다는 bool을 변경
    roomEntered = true;

    // 방 입력 창을 닫음
    gtk_window_close((GtkWindow *)_app);
}

/* sendRoomListWindow() : 방 정보를 출력하는 Window */
void showRoomListWindow(char *buffer) {
    if (strlen(buffer) <= strlen("roominfo,")) {
        // 방이 없는 경우
        logger("[ERROR] No rooms available.\n");
        return;
    }

    // 방을 출력할 컴포넌트
    GtkWidget *label;
    GtkWidget *roomNameLabel;
    GtkWidget *grid;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *window;
    gchar *text;
    char tmp[BUF_SIZE];

    strcpy(tmp, "Rooms : ");
    // 입력받은 정보를 tmp에 추가
    int j = strlen(tmp);
    for (int i = 9; buffer[i] != '\0'; i++) {
        tmp[j++] = buffer[i];
    }
    tmp[j] = '\0';

    // 방을 입력받는 Window의 타이틀, 너비 설정
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Select room");
    gtk_window_set_default_size(GTK_WINDOW(window), ROOM_WIN_WIDTH, ROOM_WIN_HEIGHT);

    // 방 입력 안내를 위한 텍스트
    label = gtk_label_new("Name : ");
    // 방 이름을 입력받기 위한 창
    roomNameLabel = gtk_label_new(tmp);
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter room name");

    // 각 구성 요소간의 여백 설정
    guint margin = 5;
    grid = gtk_grid_new();

    // 그리드의 상세 설정 (정렬 방식을 설정)
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    // 그리드를 윈도우에 추가
    gtk_container_add(GTK_CONTAINER(window), grid);

    // OK 버튼 생성
    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(sendEnterRoomRequest), (GtkWidget *)entry);

    // 버튼과 입력창 등을 그리드에 추가
    gtk_grid_attach(GTK_GRID(grid), roomNameLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 3, 1);

    // Window 띄우기
    gtk_widget_show_all(window);
}

/* createRoomRequest() : 방을 생성하기 위해 이름을 입력하는 Window */
void createRoomRequest(GtkApplication *_app, gpointer user_data) {
    // 방을 입력받기 위한 컴포넌트
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *grid;
    GtkWidget *window;
    gchar *text;

    // 방 이름을 입력받는 Window의 타이틀, 너비 설정
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Create new room");
    gtk_window_set_default_size(GTK_WINDOW(window), ROOM_WIN_WIDTH, ROOM_WIN_HEIGHT);

    // 방 이름 입력 안내를 위한 텍스트
    label = gtk_label_new("Name : ");
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "New Room");

    // 각 구성 요소간의 여백 설정
    guint margin = 5;
    grid = gtk_grid_new();

    // 그리드의 상세 설정 (정렬 방식을 설정)
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    // 그리드를 윈도우에 추가
    gtk_container_add(GTK_CONTAINER(window), grid);

    // OK 버튼 생성
    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(sendRoomRequest), (GtkWidget *)entry);

    // 버튼과 입력창 등을 그리드에 추가
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 3, 1);

    // Window 띄우기
    gtk_widget_show_all(window);
}

/* enterRoomRequest() : 방 입장을 요청하는 부분을 서버로 전송 */
void enterRoomRequest(GtkApplication *_app, gpointer user_data) {
    char *data = createMsg("roominfo", clientName, "");
    // 클라이언트 소켓을 이용하여 내용을 전송
    write(clientSocket, data, strlen(data));
}

/* buttonPressed() : 키보드가 입력됐을 경우 */
static gboolean buttonPressed(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Return) {
        // 엔터키를 눌렀을 경우
        if (strcmp(gtk_window_get_title(widget), "Login") == 0) {
            // 로그인 창은 OK 버튼을 누른것과 동일하게 해줌
            getLoginData((GtkWindow *)loginWin, NULL);
            return true;
        }

        // 채팅 Window는 Send 버튼을 누른 것과 동일하게 함
        sendText();
        return true;
    }

    return false;
}

/* mainWindow() : 기본 채팅 Window */
static void mainWindow(GtkApplication *app, gpointer user_data) {
    // 로그인 Window를 닫음
    gtk_window_close((GtkWindow *)loginWin);
    // 각 컴포넌트
    GtkWidget *window;
    GtkWidget *sendButton;
    GtkWidget *exitButton;
    GtkWidget *grid;
    GtkWidget *newRoom;
    GtkWidget *enterRoom;
    // 스레드가 반환한 값을 저장하기 위한 임시 포인터
    void *threadReturn;

    // Window의 타이틀, 너비 설정
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SimpleChat Client");
    gtk_window_set_default_size(GTK_WINDOW(window), MAIN_WIDTH, MAIN_HEIGHT);
    g_signal_connect(window, "delete-event", G_CALLBACK(closeRequest), NULL);

    // Log를 보여주는 텍스트 부분 설정
    logText = gtk_text_view_new();
    logTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logText));
    gtk_text_view_set_editable((GtkTextView *)logText, false);

    // 채팅 입력을 받는 부분 설정
    inputText = gtk_entry_new();
    gtk_entry_grab_focus_without_selecting(GTK_ENTRY(inputText));

    // 그리드에서의 요소 간의 여백 설정
    guint margin = 2;
    grid = gtk_grid_new();

    // 그리드 상세 설정
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    // Send 버튼
    sendButton = gtk_button_new_with_label("Send");
    // 클릭했을 때 작동되도록 설정
    // 메시지 전송은 스레드를 이용하지 않는다. (write만 호출하면 되기 때문)
    g_signal_connect(sendButton, "clicked", G_CALLBACK(sendText), NULL);
    // Enter 키를 눌렀을 때 작동되도록 설정
    g_signal_connect(GTK_WINDOW(window), "key_press_event", G_CALLBACK(buttonPressed), NULL);

    // Exit 버튼
    exitButton = gtk_button_new_with_label("Exit");
    g_signal_connect(exitButton, "clicked", G_CALLBACK(closeRequest), NULL);

    // 새 방 만들기 버튼
    newRoom = gtk_button_new_with_label("New\nRoom");
    g_signal_connect(newRoom, "clicked", G_CALLBACK(createRoomRequest), NULL);

    // 방 입장 버튼
    enterRoom = gtk_button_new_with_label("Enter\nRoom");
    g_signal_connect(enterRoom, "clicked", G_CALLBACK(enterRoomRequest), NULL);

    // Window에 요소 추가
    gtk_container_add(GTK_CONTAINER(window), grid);

    // 그리드에 요소 정렬
    gtk_grid_attach(GTK_GRID(grid), logText, 0, 0, 5, 5);
    gtk_grid_attach(GTK_GRID(grid), inputText, 0, 5, 5, 1);
    gtk_grid_attach(GTK_GRID(grid), exitButton, 6, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), enterRoom, 6, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), newRoom, 6, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sendButton, 6, 5, 1, 1);

    // Window를 띄움
    gtk_widget_show_all(window);

    // 서버에 접속하는 스레드 생성
    pthread_create(&connectThread, NULL, connectServer, NULL);
    // 접속할 때까지 Blocking 하는 join 호출
    pthread_join(connectThread, &threadReturn);

    // 서버에서 데이터를 받는 스레드 생성
    // 이는 스레드를 이용하지 않으면, 다른 요청이 없는 한 프로그램이 멈춰있게 된다.
    pthread_create(&receiveThread, NULL, receiveData, NULL);
    // Blocking 해제하는 detach, join을 이용할 경우 blocking 상태가 되므로 detach를 이용한다.
    pthread_detach(receiveThread);
}

/* loginWindow() : 서버 IP, 포트 번호, 이름을 입력받는 Window */
static void loginWindow(GtkApplication *app, gpointer user_data) {
    // 컴포넌트
    GtkWidget *button;
    // 3개의 입력 창
    GtkWidget *inputLabels[3];
    GtkWidget *grid;
    gchar *text;

    // 새 Window 설정
    loginWin = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(loginWin), "Login");
    gtk_window_set_default_size(GTK_WINDOW(loginWin), LOGIN_WIN_WIDTH, LOGIN_WIN_HEIGHT);
    // Enter 키를 눌렀을 때 작동 설정
    g_signal_connect(GTK_WINDOW(loginWin), "key_press_event", G_CALLBACK(buttonPressed), NULL);

    // IP 입력 부분
    inputLabels[0] = gtk_label_new("IP : ");
    inputEntries[0] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(inputEntries[0]), "127.0.0.1");
    gtk_entry_set_text(GTK_ENTRY(inputEntries[0]), "127.0.0.1");

    // 포트 입력 부분
    inputLabels[1] = gtk_label_new("Port : ");
    inputEntries[1] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(inputEntries[1]), "7777");
    gtk_entry_set_text(GTK_ENTRY(inputEntries[1]), "7777");

    // 이름 입력 부분
    inputLabels[2] = gtk_label_new("Name : ");
    inputEntries[2] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(inputEntries[2]), "Name");
    gtk_entry_set_text(GTK_ENTRY(inputEntries[2]), "Name");

    // 그리드에서의 요소 간의 여백 설정
    guint margin = 5;
    grid = gtk_grid_new();

    // 그리드 상세 설정
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), margin);
    gtk_grid_set_column_spacing(GTK_GRID(grid), margin);

    // 그리드 추가
    gtk_container_add(GTK_CONTAINER(loginWin), grid);

    // OK 버튼
    button = gtk_button_new_with_label("OK");
    g_signal_connect(button, "clicked", G_CALLBACK(getLoginData), loginWin);

    // 그리드에 정렬
    gtk_grid_attach(GTK_GRID(grid), inputLabels[0], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputEntries[0], 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), inputLabels[1], 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputEntries[1], 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), inputLabels[2], 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), inputEntries[2], 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 3, 1);

    // Window 띄우기
    gtk_widget_show_all(loginWin);
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