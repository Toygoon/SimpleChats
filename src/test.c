#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1000

static WINDOW *logFrame, *cmdFrame;

int main(int argc, char *argv[]) {
    // width, height : 가로, 세로 크기 (문자 단위)
    // cmdFrameLeftMargin : 명령어 입력 WINDOW의 왼쪽 크기
    int width, height, cmdFrameLeftMargin = 3, logFrameCurrent = 0;

    // 명령어 입력에 사용될 버퍼
    char buffer[BUF_SIZE];

    // 화면 초기화, 설정
    initscr();

    // 현재 Terminal의 크기 계산
    getmaxyx(stdscr, height, width);
    int logFrameMaxLine = height - cmdFrameLeftMargin - 2;

    // Frame 설정
    logFrame = newwin(height - cmdFrameLeftMargin, width, 0, 0);
    cmdFrame = newwin(cmdFrameLeftMargin, width, height - cmdFrameLeftMargin, 0);

    // 각 Frame에 테두리 적용
    box(logFrame, 0, 0);
    box(cmdFrame, 0, 0);

    // 환영 메시지 출력
    mvwprintw(logFrame, ++logFrameCurrent, 1, "Field");

    // 정의한 Frame을 출력하도록 설정
    wrefresh(stdscr);
    wrefresh(logFrame);

    // exit가 입력될 때까지 반복
    while (strcmp(buffer, "exit") != 0) {
        // 명령어 입력 WINDOW 설정
        // printf와 동일
        mvwprintw(cmdFrame, 1, 1, " >> ");
        // 기존에 입력된 명령어가 있다면 삭제하도록 redraw window
        redrawwin(cmdFrame);
        // Frame 출력
        wrefresh(cmdFrame);
        // 입력할 커서의 위치를 설정
        move(height - cmdFrameLeftMargin + 1, 5);
        // fgets와 동일, 문자열 입력
        getstr(buffer);

        // logFrame이 가득차면 기존 WINDOW 삭제 후 초기화
        if (logFrameCurrent >= logFrameMaxLine) {
            logFrame = newwin(height - cmdFrameLeftMargin, width, 0, 0);
            box(logFrame, 0, 0);

            logFrameCurrent = 0;
        }

        // logFrame에 출력
        mvwprintw(logFrame, ++logFrameCurrent, 1, "%s", buffer);
        wrefresh(logFrame);
    }

    // 종료
    endwin();
    return 0;
}