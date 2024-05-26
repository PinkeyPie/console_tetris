#include <stdio.h>
#include "Collection.h"
#include "mem.h"
#include "stdlib.h"

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
    DWORD dwList = CreateList();
    for(int i = 0; i < 10; i++) {
        AddIntElement(dwList, i);
    }
    SetInt(dwList, 0, 10);
    SetInt(dwList, 4, 14);
    RemoveAt(dwList, 5);
    InsertIntAt(dwList, 5, 15);
    RemoveAt(dwList, 0);
    for(int i = 0; i < Size(dwList); i++) {
        int item = GetIntAt(dwList, i);
        printf("item: %d\n", item);
    }
    DWORD dwVector = CreateVector(Int, 0);
    for(int i = 0; i < 10; i++) {
        AddIntElement(dwVector, i);
    }
    SetInt(dwVector, 0, 10);
    SetInt(dwVector, 4, 14);
    RemoveAt(dwVector, 5);
    InsertIntAt(dwVector, 5, 15);
    RemoveAt(dwVector, 0);
    for(int i = 0; i < Size(dwVector); i++) {
        printf("Vector item: %d\n", GetIntAt(dwVector,i));
    }
    DWORD figuresList = CreateList();
    TetrisFigure figure;
    figure.x = 10;
    figure.y = 0;
    figure.figureType = LeftZ;
    AddElement(figuresList, &figure, sizeof(TetrisFigure));
    for(int i = 0; i < Size(figuresList); i++) {
        TetrisFigure* pFigure = (TetrisFigure*)GetAt(figuresList, i);
        printf("Figure{%d, %d}", pFigure->x, pFigure->y);
    }
    FreeCollections();
    return 0;
}