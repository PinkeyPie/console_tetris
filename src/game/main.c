#include <malloc.h>
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

typedef struct myStruct {
    char* name;
    char* surname;
    int height;
} myStruct;

myStruct* createStruct(const char* n, const char* s, int h) {
    if(h > 300) {
        h = 0;
    }
    myStruct aStruct;
    aStruct.name = (char*)malloc((strlen(n)+1)*sizeof(char));
    strcpy(aStruct.name, n);
    aStruct.surname = (char*) malloc((strlen(s) + 1)*sizeof(char));
    strcpy(aStruct.surname, s);
    aStruct.height = h;
    return &aStruct;
}

void setMyStruct(myStruct** aStruct) {
    myStruct local;
    local.name = "Alyosha";
    local.surname = "Viktorovich";
    local.height = 182;
    *aStruct = &local;
}

int main(int argc, char* argv[]) {
    XInitThreads();
    myStruct* str = NULL;
    setMyStruct(&str);

    if (!InitScreen(argc, argv)) {
        printf("Error: cant initialize screen");
        return -1;
    }

    pthread_t graphical_thread;
    pthread_t x11Thread;

    int status = pthread_create(&graphical_thread, NULL, GraphicalThread, NULL) |
                 pthread_create(&x11Thread, NULL, X11EventHandler, NULL);
    if(status != 0) {
        printf("Error: cant initialize graphical loop");
        return -1;
    }

    GameLoop(NULL);
    pthread_join(graphical_thread, NULL);
    pthread_join(x11Thread, NULL);
    DisposeScreen();
    return 0;
}
