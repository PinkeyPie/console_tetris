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
    return NULL;
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

//HANDLE hSharedList = NULL;
//pthread_mutex_t mutex;
//pthread_cond_t cond_var;
//
//void* thread1(void* params) {
//    for(int i = 0; i < 10; i++) {
//        TetrisFigure* pFigure = CreateRandomFigure();
//        DrawMessage drawMessage;
//        drawMessage.drawTarget = EFigureMove;
//        drawMessage.figureMove.oldPosition = CopyFigure(pFigure);
//        pFigure->coords.Y++;
//        drawMessage.figureMove.newPosition = CopyFigure(pFigure);
//        pthread_mutex_lock(&mutex);
//        AddElement(hSharedList, &drawMessage, sizeof(drawMessage));
//        pthread_mutex_unlock(&mutex);
//        pthread_cond_broadcast(&cond_var);
//        printf("Added element to list\n");
//        FreeFigure(pFigure);
//        sleep(1);
//    }
//
//    return NULL;
//}
//
//void* thread2(void* params) {
//    HANDLE hLocal = CreateList();
//    int counter = 0;
//    while (counter < 10) {
//        pthread_mutex_lock(&mutex);
//        if (Size(hSharedList) == 0) {
//            pthread_cond_wait(&cond_var, &mutex);
//        }
//        while (Size(hSharedList) != 0) {
//            HANDLE hMessage = GetAt(hSharedList, 0);
//            AddElement(hLocal, hMessage, sizeof(GameMessage));
//            RemoveAt(hSharedList, 0);
//            printf("Removed element from list\n");
//        }
//        pthread_mutex_unlock(&mutex);
//        while (Size(hLocal) != 0) {
//            DrawMessage *message = GetAt(hLocal, 0);
//            FreeFigure(message->figureMove.oldPosition);
//            FreeFigure(message->figureMove.newPosition);
//            RemoveAt(hLocal, 0);
//            counter++;
//            printf("Removed element from local list\n");
//        }
//    }
//    DeleteList(hLocal);
//
//    return NULL;
//}

int main(int argc, char* argv[]) {
//    HANDLE hList = CreateList();
//    HANDLE hOther = CreateList();
//    TetrisFigure* pFigure = CreateRandomFigure();
//    DrawMessage drawMessage;
//    drawMessage.drawTarget = EFigureMove;
//    drawMessage.figureMove.oldPosition = CopyFigure(pFigure);
//    pFigure->coords.Y++;
//    drawMessage.figureMove.newPosition = CopyFigure(pFigure);
//    AddElement(hList, &drawMessage, sizeof(drawMessage));
//    while (Size(hList) != 0) {
//        HANDLE hMessage = GetAt(hList, 0);
//        AddElement(hOther, hMessage, sizeof(DrawMessage));
//        RemoveAt(hList, 0);
//    }
//    DrawMessage* pFig = GetAt(hOther, 0);
//    FreeFigure(pFig->figureMove.oldPosition);
//    FreeFigure(pFig->figureMove.newPosition);
//    FreeFigure(pFigure);
//    DeleteList(hOther);
//    DeleteList(hList);


    XInitThreads();
//    int shit = 0;
//
//    HANDLE hList = CreateList();
//    for (int i = 0; i < 10; i++)
//    {
//        AddElement(hList, &shit, sizeof(int));
//        RemoveAt(hList,0);
//    }
//    DeleteList(hList);



//    pthread_t _thread1;
//    pthread_t _thread2;
//    hSharedList = CreateList();
//    int status = pthread_create(&_thread1, NULL, thread1, NULL) | pthread_create(&_thread2, NULL, thread2, NULL);
//    pthread_join(_thread1, NULL);
//    pthread_join(_thread2, NULL);
//    DeleteList(hSharedList);

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
