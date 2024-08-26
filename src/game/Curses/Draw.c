//
// Created by leon on 26.08.24.
//

#include "Draw.h"
#include "ncurses.h"
#include "pthread.h"
#include "panel.h"
#include "sys/ioctl.h"
#include "signal.h"
#include "termios.h"
#include "../GameLogic.h"

#define WIDTH 120
#define HEIGHT 70

#define NLINES 10
#define NCOLS 40

void init_wins(WINDOW** wins, int n) {
    int x, y, i;

    y = 2;
    x = 10;

    for(i = 0; i < n; ++i) {
        wins[i] = newwin(NLINES, NCOLS, y, x);
        mvwhline(wins[i], 0, 3, ACS_HLINE, 10);
        wbkgdset(wins[i], COLOR_PAIR(i + 1));
        wclear(wins[i]);
        wrefresh(wins[i]);

        y += 3;
        x += 7;
    }
}

void handle_winch(int sig) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    COLS = w.ws_col;
    LINES = w.ws_row;

    wresize(stdscr, LINES, COLS);
    clear();

    mvprintw(0, 0, "COLS = %d, LINES = %d", COLS, LINES);
    for(int i = 0; i < COLS; i++) {
        mvaddch(1, i, '*');
    }
    refresh();
}

BOOL InitScreen(int argc, char* argv[]) {
    WINDOW *my_wins[3];
    PANEL *my_panels[3];
    PANEL *top;
    int ch;
    int i;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);

    if(!has_colors()) {
        endwin();
        printf("Error! Colors not supported");
        return FALSE;
    }
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLUE);
    init_pair(2, COLOR_GREEN, COLOR_RED);
    init_pair(3, COLOR_BLUE, COLOR_GREEN);

    init_wins(my_wins, 3);
    my_panels[0] = new_panel(my_wins[0]);
    my_panels[1] = new_panel(my_wins[1]);
    my_panels[2] = new_panel(my_wins[2]);

    set_panel_userptr(my_panels[0], my_panels[1]);
    set_panel_userptr(my_panels[1], my_panels[2]);
    set_panel_userptr(my_panels[2], my_panels[0]);

    update_panels();
    mvprintw(0, 0, "Tab - next panel f1 - exit");
    doupdate();
    top = my_panels[2];


//    int row, col;
//    getmaxyx(stdscr, row, col);
//    move(row / 2, col / 2);
//    printw("Hello world");
//    refresh();
//    getch();
//    endwin();
//    addch('f');


    return TRUE;
}

void DrawLoop() {

}

void* DrawEventHandler() {
    while(TRUE) {
        int key = getch();
        GameMessage gameMessage;
        gameMessage.type = EControlMessage;
        ControlMessage controlMessage;
        controlMessage.InputData.keycode = key;
        gameMessage.messageInfo.controlMessage = controlMessage;
        PutControlMessage(&gameMessage);
    }
    printf("End event loop\n");
    pthread_exit(NULL);
}

void DisposeScreen() {

}

void PutDrawMessage(HANDLE hMessage) {

}