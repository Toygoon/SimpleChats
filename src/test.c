#include <curses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int parent_x, parent_y, new_x, new_y;
    int score_size = 3;
    // 명령어 입력에 사용될 버퍼
    char buffer[100];

    // 화면 초기화, 설정
    initscr();

    // 현재 Terminal의 크기 계산
    getmaxyx(stdscr, parent_y, parent_x);

    WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0);
    WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0);

    box(field, 0, 0);
    box(score, 0, 0);

    // draw to our windows
    mvwprintw(field, 1, 1, "Field");
    
    // refresh each window
    wrefresh(stdscr);
    wrefresh(field);

    while (strcmp(buffer, "quit") != 0) {
        mvwprintw(score, 1, 1, " >> ");
        redrawwin(score);
        wrefresh(score);
        move(parent_y - score_size + 1, 5);
        getstr(buffer);
    }

    endwin();

    return 0;
}