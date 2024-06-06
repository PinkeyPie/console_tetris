#include <stdio.h>
#include "Collection.h"
#include "stdlib.h"
#ifdef WIN32
#include "windows.h"
#include "conio.h"
#endif
#include "Draw.h"

typedef enum TetrisFigureTypes {
    Stick = 1,
    Rect = 2,
    TFigure = 3,
    RightZ = 4,
    LeftZ = 5,
    LeftL = 6,
    RightL = 7
} TetrisFigureTypes;

typedef struct TetrisFigure {
    int x;
    int y;
    TetrisFigureTypes figureType;
} TetrisFigure;

void Destruct(void* element) {


}

void (*destroy)(void*);

int main() {
    //DrawGameWindow();
    DWORD dwLines = CreateVector(EString, sizeof(wchar_t));
    AddStringElement(dwLines, L"Hello world");
    wchar_t* szLine = GetStringAt(dwLines, 0);

    while (TRUE)
    {
        int key = _getch();
        SetUserKey(key);
        Sleep(1000 / 60);
    };
    FreeCollections();
    return 0;
}