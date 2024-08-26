#include <malloc.h>
#ifdef CURSES_LIB
#include "Curses/Draw.h"
#else
#include "X11/Draw.h"
#endif
#include "GameLogic.h"
#include "Collection.h"
#include "pthread.h"
#include "TetrisFigure.h"
#include "StringLib.h"
#include "TwoDimArray.h"

void* GraphicalThread(void* args) {
    DrawLoop();
    return NULL;
}

int main(int argc, char* argv[]) {
#ifndef CURSES_LIB
    XInitThreads();
#endif
    if (!InitScreen(argc, argv)) {
        printf("Error: cant initialize screen");
        return -1;
    }

    pthread_t graphical_thread;
    pthread_t eventThread;

    int status = pthread_create(&graphical_thread, NULL, GraphicalThread, NULL) |
                 pthread_create(&eventThread, NULL, DrawEventHandler, NULL);
    if(status != 0) {
        printf("Error: cant initialize graphical loop");
        return -1;
    }

    GameLoop(NULL);
    pthread_join(graphical_thread, NULL);
    pthread_join(eventThread, NULL);
    DisposeScreen();
    return 0;
}
