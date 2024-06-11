#include "Draw.h"
#include "GameLogic.h"
#include "Collection.h"
#include "pthread.h"
#include "TetrisFigure.h"
#include "StringLib.h"
#include "TwoDimArray.h"

void* GraphicalThread(void* args) {
    DrawLoop();
}

void printArray(TwoDim* array) {
    for(int i = 0; i < array->height; i++) {
        for(int j = 0; j < array->width; j++) {
            unsigned char ch = Get(array, j, i);
            if(ch == 0) {
                printf("0");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (!InitScreen(argc, argv)) {
        printf("Error: cant initialize screen");
        return -1;
    }

    pthread_t graphical_thread;
    pthread_t x11Thread;
    pthread_t gameThread;
    int status = pthread_create(&graphical_thread, NULL, GraphicalThread, NULL) |
                 pthread_create(&x11Thread, NULL, X11EventHandler, NULL) |
                 pthread_create(&gameThread, NULL, GameLoop, NULL);
    if(status != 0) {
        printf("Error: cant initialize graphical loop");
        return -1;
    }

    while (True) { // Game logic loop
        usleep(10000);
    }
    pthread_join(graphical_thread, NULL);
    DisposeScreen();
    FreeCollections();
    return 0;
}
