/*
 * Simple ncurses form example with fields that actually behaves like fields.
 *
 * How to run:
 *	gcc -Wall -Werror -g -pedantic -o test fields_magic.c -lform -lncurses
 */

#include <assert.h>
#include <ctype.h>
#include <form.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static FORM *form;
static FIELD *fields[5];
static WINDOW *winFrame, *winForm;
struct winsize windowSize;

static char *trim_whitespaces(char *);
static void driver(int);

int main() {
    // 출력 메시지에 사용될 버퍼
    char *msg;

    // 현재 Terminal의 크기 계산
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);

    // ch : 키보드 입력 값을 코드로 받아 저장하는 변수
    // width, height : 가로, 세로 크기 (문자 단위)
    // locX, locY : 가로, 세로 크기를 통해 중간에 위치하게 함
    int ch, width = 24, height = 60, locX = (windowSize.ws_col - height) / 2, locY = (windowSize.ws_row - width) / 2;

    // 화면 초기화, 설정
    initscr();
    // 화면에 입력하는 값을 출력하지 않도록 설정
    noecho();
    // cbreak 모드 사용
    cbreak();
    // 키보드의 입력을 받아들일 때 접두사 KEY_ 형태로 받아들이게 함
    keypad(stdscr, TRUE);

    // Frame 설정
    winFrame = newwin(width, height, locY, locX);
    // 현재 Frame에 테두리 적용
    box(winFrame, 0, 0);
    // 전체 Frame 내부의 Form 생성
    winForm = derwin(winFrame, 20, 40, locY, locX);
    
    // 환영 메시지 출력
    mvwprintw(winFrame, 1, 13, "-- Welcome to SimpleChat Server --");

    fields[0] = new_field(1, 10, 0, 0, 0, 0);
	fields[1] = new_field(1, 40, 0, 15, 0, 0);
	fields[2] = new_field(1, 10, 2, 0, 0, 0);
	fields[3] = new_field(1, 40, 2, 15, 0, 0);
	fields[4] = NULL;
	assert(fields[0] != NULL && fields[1] != NULL && fields[2] != NULL && fields[3] != NULL);

	set_field_buffer(fields[0], 0, "label1");
	set_field_buffer(fields[1], 0, "val1");
	set_field_buffer(fields[2], 0, "label2");
	set_field_buffer(fields[3], 0, "val2");

	set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
	set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[3], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

	set_field_back(fields[1], A_UNDERLINE);
	set_field_back(fields[3], A_UNDERLINE);

	form = new_form(fields);
	set_form_sub(form, winForm);

    // Form 생성
	post_form(form);

    // 메모리 버퍼에 남아있는 Form과 관련된 내용 Flush
	refresh();
	wrefresh(winFrame);


    while ((ch = getch()) != KEY_F(1))
        driver(ch);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    free_field(fields[1]);
    free_field(fields[2]);
    free_field(fields[3]);
    delwin(winForm);
    delwin(winFrame);
    endwin();

    return 0;
}


/*
 * This is useful because ncurses fill fields blanks with spaces.
 */
static char *trim_whitespaces(char *str) {
    char *end;

    // trim leading space
    while (isspace(*str))
        str++;

    if (*str == 0)  // all spaces?
        return str;

    // trim trailing space
    end = str + strnlen(str, 128) - 1;

    while (end > str && isspace(*end))
        end--;

    // write new null terminator
    *(end + 1) = '\0';

    return str;
}

static void driver(int ch) {
    int i;

    switch (ch) {
        case KEY_F(2):
            // Or the current field buffer won't be sync with what is displayed
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_PREV_FIELD);
            move(LINES - 3, 2);

            for (i = 0; fields[i]; i++) {
                printw("%s", trim_whitespaces(field_buffer(fields[i], 0)));

                if (field_opts(fields[i]) & O_ACTIVE)
                    printw("\"\t");
                else
                    printw(": \"");
            }

            refresh();
            pos_form_cursor(form);
            break;

        case KEY_DOWN:
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_END_LINE);
            break;

        case KEY_UP:
            form_driver(form, REQ_PREV_FIELD);
            form_driver(form, REQ_END_LINE);
            break;

        case KEY_LEFT:
            form_driver(form, REQ_PREV_CHAR);
            break;

        case KEY_RIGHT:
            form_driver(form, REQ_NEXT_CHAR);
            break;

        // Delete the char before cursor
        case KEY_BACKSPACE:
        case 127:
            form_driver(form, REQ_DEL_PREV);
            break;

        // Delete the char under the cursor
        case KEY_DC:
            form_driver(form, REQ_DEL_CHAR);
            break;

        default:
            form_driver(form, ch);
            break;
    }

    wrefresh(winForm);
}
