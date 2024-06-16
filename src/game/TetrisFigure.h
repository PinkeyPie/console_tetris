//#pragma once
//#include "types.h"
//#ifdef WIN32
//#include "windows.h"
//#endif
//
//#define HAS_NEXT 1
//#define TRAVERSE_END 0
//
//typedef enum Positions
//{
//    EUp,
//    ELeft,
//    ERight,
//    EDown
//} Positions;
//
//typedef enum EFigure {
//    LFigure,
//    TFigure,
//    ZFigure,
//    RectFigure,
//    Stick,
//    ZReversed,
//    LReversed
//} EFigure;
//
//typedef struct TetrisFigure
//{
//	COORD coords;
//    Positions position;
//    EFigure figureType;
//    PCHAR szFigureText;
//} TetrisFigure;
//
//HANDLE CreateTetrisFigure(EFigure figureType);
//void SetTetrisFigureCoordinates(HANDLE hFigure, COORD coords);
//void ChangeOrientation(HANDLE hFigure, Positions position);
//HANDLE InitCollisionTraverse(HANDLE hFigure);
//DWORD GetCollisionBoundPixel(HANDLE hIterator, COORD* pPixelCoord);
//BOOL GetFigurePixels(DWORD dwPixelsList);