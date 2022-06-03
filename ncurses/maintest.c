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

static FORM *form;
static FIELD *fields[5];
static WINDOW *win_body, *win_form;

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

    wrefresh(win_form);
}

int main() {
    // 키보드 입력 값으로 사용될 코드
    int ch;

    // 화면 초기화, 설정
    initscr();
    noecho();
    cbreak();
    // 숫자 키패드 사용
    keypad(stdscr, TRUE);

    // 전체 Body 설정
    win_body = newwin(24, 80, 0, 0);
    box(win_body, 0, 0);
    win_form = derwin(win_body, 20, 78, 3, 1);
    assert(win_form != NULL);
    box(win_form, 0, 0);
    mvwprintw(win_body, 1, 2, "Press F1 to quit and F2 to print fields content");

    while ((ch = getch()) != KEY_F(1))
        driver(ch);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    free_field(fields[1]);
    free_field(fields[2]);
    free_field(fields[3]);
    delwin(win_form);
    delwin(win_body);
    endwin();

    return 0;
}