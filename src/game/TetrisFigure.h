#pragma once
#include "types.h"
#ifdef WIN32
#include "windows.h"
#endif

#define HAS_NEXT 1
#define TRAVERSE_END 0

typedef enum Positions
{
    EUp = 0,
    ELeft,
    ERight,
    EDown
} Positions;

typedef enum EFigure {
    LFigure = 1,
    TFigure,
    ZFigure,
    RectFigure,
    Stick,
    ZReversed,
    LReversed
} EFigure;

typedef struct TetrisFigure
{
    COORD coords;
    Positions position;
    EFigure figureType;
    HANDLE hCoordsList;
} TetrisFigure;

HANDLE CreateTetrisFigure(EFigure figureType);
HANDLE CreateRandomFigure();
void SetTetrisFigureCoordinates(HANDLE hFigure, COORD coords);
void ChangeOrientation(HANDLE hFigure);
HANDLE InitCollisionTraverse(HANDLE hFigure);
DWORD GetCollisionBoundPixel(HANDLE hIterator, COORD* pPixelCoord);
BOOL GetFigurePixels(DWORD dwPixelsList);
HANDLE CopyFigure(HANDLE hFigure);
void FreeFigure(HANDLE hFigure);
COORD GetFigureSizes(HANDLE hFigure);