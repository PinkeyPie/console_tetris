#include <stdio.h>
#include "Collection.h"
#include "stdlib.h"
#ifdef WIN32
#include "windows.h"
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

int main() {
    //DrawGameWindow();
    DWORD dwSize = 130 * sizeof(TCHAR);
    PTCHAR szOutput = HeapAlloc(GetProcessHeap(), 0, dwSize);
    szOutput[0] = '\0';
    strcat_s(szOutput, dwSize, "+");
    strcat_s(szOutput, dwSize, "-");
    strcat_s(szOutput, dwSize, "-");
    strcat_s(szOutput, dwSize, "+");
    printf_s("%s", szOutput);
    HeapFree(GetProcessHeap(), 0 , szOutput);

    getchar();

    DWORD dwList = CreateList();
    for(int i = 0; i < 10; i++) {
        TetrisFigure figure;
        figure.x = 0;
        figure.y = i;
        figure.figureType = Stick;
        AddElement(dwList, &figure, sizeof(TetrisFigure));
    }
    for(int i = 0; i < 10; i++) {
        TetrisFigure* pElement = (TetrisFigure*)GetAt(dwList, i);
        printf_s("Figure(%d, %d)\n", pElement->x, pElement->y);
    }
    printf_s("Hello world");
    FreeCollections();
    return 0;
}