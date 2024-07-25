#include "TetrisFigure.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Collection.h"
#include <time.h>

#ifndef WIN32
#define wsprintf sprintf
#define HeapAlloc(HEAP, FLAGS, SIZE) malloc(SIZE)
#define strcpy_s(DST, SIZE, SRC) strcpy(DST,SRC)
#define HeapFree(HEAP, FLAGS, PTR) free(PTR)
#define VOID void
#define memcpy_s(DST, DST_SIZE, PTR, SIZE) memcpy(DST, PTR, SIZE)
#define GetProcessHeap() 0
#endif

static BOOL randomInited = FALSE;

typedef struct PixelIterator {
    COORD currentCoord;
    DWORD iteraton;
    HANDLE tetrisFigure;
    HANDLE actionsArray;
    DWORD actionsSize;
    DWORD nActionsSize;
    DWORD nCurrentAction;
} PixelIterator;

size_t GetCharCount(char *szLine, char ch) {
    size_t size = strlen(szLine);
    size_t retSize = 0;
    for (size_t i = 0; i < size; i++) {
        if (szLine[i] == ch) {
            retSize++;
        }
    }
    return retSize;
}

size_t GetLineSize(char *szLine, size_t line) {
    size_t result = 0;
    size_t lineSize = strlen(szLine);
    int i = 0, j = 0;
    while (j < lineSize) {
        if (i == line) {
            result++;
        }
        if (szLine[j] == '\n') {
            i++;
            if (i > line) {
                break;
            }
        }
        j++;
    }
    return result;
}

BOOL GetCoords(char *szLine, HANDLE hCoordsList) {
    size_t linesCount = GetCharCount(szLine, '\n');
    COORD coord = {0, 0};
    size_t lineSize = GetLineSize(szLine, 0);
    for (size_t i = 0; i < linesCount; i++) {
        for (size_t j = 0; j < lineSize; j++) {
            if (szLine[i * lineSize + j] != ' ' && szLine[i * lineSize + j] != '\n') {
                coord.X = (short) j;
                coord.Y = (short) i;
                AddElement(hCoordsList, &coord, sizeof(COORD));
            }
        }
    }
    return TRUE;
}

HANDLE CreateTetrisFigure(EFigure figureType) {
    TetrisFigure *pFigure = malloc(sizeof(TetrisFigure));
    char szBufferText[20];
    HANDLE hCoords = CreateVector(EStruct, sizeof(COORD));
    switch (figureType) {
        case TFigure:
            wsprintf(szBufferText, " * \n***\n");
            break;
        case LFigure:
            wsprintf(szBufferText, "* \n* \n**\n");
            break;
        case ZFigure:
            wsprintf(szBufferText, "** \n **\n");
            break;
        case RectFigure:
            wsprintf(szBufferText, "**\n**\n");
            break;
        case Stick:
            wsprintf(szBufferText, "*\n*\n*\n*\n");
            break;
        case ZReversed:
            wsprintf(szBufferText, " **\n** \n");
            break;
        case LReversed:
            wsprintf(szBufferText, " *\n *\n**\n");
            break;
        default:
            break;
    }
    GetCoords(szBufferText, hCoords);
    pFigure->position = EUp;
    pFigure->hCoordsList = hCoords;
    pFigure->figureType = figureType;

    return pFigure;
}

HANDLE CreateRandomFigure() {
    if (!randomInited) {
        srand(time(NULL));   // Initialization, should only be called once.
        randomInited = TRUE;
    }
    EFigure randNumber = (EFigure) ((rand() % 7) + 1);      // Returns a pseudo-random integer between 0 and RAND_MAX.
    return CreateTetrisFigure(randNumber); //
}

COORD GetFigureSizes(HANDLE hFigure) {
    COORD retCoord = {0, 0};
    TetrisFigure* pFigure = (TetrisFigure*)hFigure;
    // T up down L right left ~L right left Z up down ~Z up down -> 3x2
    // T left right L up down ~L up down Z left right ~Z left right -> 2x3
    // Rect 2x2
    // Stick up down -> 4x1
    // Stick left right = 1x4
    EFigure t = pFigure->figureType;
    Positions p = pFigure->position;
    if(((t == TFigure || t == ZFigure || t == ZReversed) && (p == EUp || p == EDown)) ||
       ((t == LFigure || t == LReversed) && (p == ERight || p == ELeft))) {
        retCoord.X = 3;
        retCoord.Y = 2;
    }
    if(((t == TFigure || t == ZFigure || t == ZReversed) && (p == ELeft || p == ERight)) ||
       ((t == LFigure || t == LReversed) && (p == EDown || p == EUp))) {
        retCoord.X = 2;
        retCoord.Y = 3;
    }
    if(t == RectFigure) {
        retCoord.X = 2;
        retCoord.Y = 2;
    }
    if(t == Stick && (p == EUp || p == EDown)) {
        retCoord.X = 1;
        retCoord.Y = 4;
    }
    if(t == Stick && (p == ERight || p == ELeft)) {
        retCoord.X = 4;
        retCoord.Y = 1;
    }
    return retCoord;
}

void ChangeOrientation(HANDLE hFigure) {
    TetrisFigure *pFigure = (TetrisFigure *) hFigure;
    char szBuffer[20];
    DeleteList(pFigure->hCoordsList);
    HANDLE hNewCoords = CreateVector(EStruct, sizeof(COORD));
    Positions position = pFigure->position;
    switch (pFigure->figureType) {
        case LFigure:
            switch (position) {
                case ELeft:
                    wsprintf(szBuffer, "* \n* \n**\n");
                    pFigure->coords.X++;
                    position = EUp;
                    break;
                case EUp:
                    wsprintf(szBuffer, "***\n*  \n");
                    pFigure->coords.Y++;
                    pFigure->coords.X--;
                    position = ERight;
                    break;
                case ERight:
                    wsprintf(szBuffer, "**\n *\n *\n");
                    pFigure->coords.Y--;
                    position = EDown;
                    break;
                case EDown:
                    wsprintf(szBuffer, "  *\n***\n");
                    position = ELeft;
                    break;
                default:
                    break;
            }
            break;
        case TFigure:
            switch (position) {
                case ELeft:
                    wsprintf(szBuffer, " * \n***\n");
                    position = EUp;
                    break;
                case EUp:
                    wsprintf(szBuffer, "* \n**\n* \n");
                    pFigure->coords.X++;
                    position = ERight;
                    break;
                case ERight:
                    wsprintf(szBuffer, "***\n * \n");
                    pFigure->coords.Y++;
                    pFigure->coords.X--;
                    position = EDown;
                    break;
                case EDown:
                    wsprintf(szBuffer, " *\n**\n *\n");
                    pFigure->coords.Y--;
                    position = ELeft;
                    break;
                default:
                    break;
            }
            break;
        case ZFigure:
            switch (position) {
                case ELeft:
                    wsprintf(szBuffer, "** \n **\n");
                    pFigure->coords.X--;
                    position = EUp;
                    break;
                case EUp:
                    wsprintf(szBuffer, " *\n**\n* \n");
                    pFigure->coords.X++;
                    position = ERight;
                    break;
                case ERight:
                    wsprintf(szBuffer, "** \n **\n");
                    position = EDown;
                    break;
                case EDown:
                    wsprintf(szBuffer, " *\n**\n* \n");
                    position = ELeft;
                    break;
                default:
                    break;
            }
            break;
        case RectFigure:
            switch (position) {
                case EUp:
                case EDown:
                case ELeft:
                case ERight:
                    wsprintf(szBuffer, "**\n**\n");
                    position = EUp;
                    break;
                default:
                    break;
            }
            break;
        case Stick:
            switch (position) {
                case ELeft:
                    wsprintf(szBuffer, "*\n*\n*\n*\n");
                    pFigure->coords.Y--;
                    pFigure->coords.X++;
                    position = EUp;
                    break;
                case EUp:
                    wsprintf(szBuffer, "****\n");
                    pFigure->coords.X--;
                    pFigure->coords.Y++;
                    position = ERight;
                    break;
                case ERight:
                    wsprintf(szBuffer, "*\n*\n*\n*\n");
                    pFigure->coords.X += 2;
                    pFigure->coords.Y--;
                    position = EDown;
                    break;
                case EDown:
                    wsprintf(szBuffer, "****\n");
                    pFigure->coords.X -= 2;
                    pFigure->coords.Y++;
                    position = ELeft;
                    break;
                default:
                    break;
            }
            break;
        case ZReversed:
            switch (position) {
                case ELeft:
                    wsprintf(szBuffer, " **\n** \n");
                    pFigure->coords.X--;
                    position = EUp;
                    break;
                case EUp:
                    wsprintf(szBuffer, "* \n**\n *\n");
                    pFigure->coords.X++;
                    position = ERight;
                    break;
                case ERight:
                    wsprintf(szBuffer, " **\n** \n");
                    position = EDown;
                    break;
                case EDown:
                    wsprintf(szBuffer, "* \n**\n *\n");
                    position = ELeft;
                    break;
                default:
                    break;
            }
            break;
        case LReversed:
            switch (position) {
                case ELeft:
                    wsprintf(szBuffer, " *\n *\n**\n");
                    pFigure->coords.X++;
                    position = EUp;
                    break;
                case EUp:
                    wsprintf(szBuffer, "*  \n***\n");
                    pFigure->coords.X--;
                    pFigure->coords.Y++;
                    position = ERight;
                    break;
                case ERight:
                    wsprintf(szBuffer, "**\n* \n* \n");
                    pFigure->coords.Y--;
                    position = EDown;
                    break;
                case EDown:
                    wsprintf(szBuffer, "***\n  *\n");
                    position = ELeft;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    pFigure->position = position;
    BOOL result = GetCoords(szBuffer, hNewCoords);
    pFigure->hCoordsList = hNewCoords;
}

HANDLE InitCollisionTraverse(HANDLE hFigure) {
    TetrisFigure *pFigure = (TetrisFigure *) hFigure;
    if (pFigure == NULL) {
        return NULL;
    }
#ifdef WIN32
    PixelIterator* iterator = HeapAlloc(GetProcessHeap(), 0, sizeof(PixelIterator));
#else
    PixelIterator *iterator = malloc(sizeof(PixelIterator));
#endif
    if (iterator != NULL) {
        iterator->tetrisFigure = pFigure;
        iterator->iteraton = 0;
        iterator->currentCoord.X = 0;
        iterator->currentCoord.Y = 0;
        switch (pFigure->figureType) {
            case TFigure:
                iterator->actionsArray = HeapAlloc(GetProcessHeap(), 0, 8 * sizeof(COORD));
                switch (pFigure->position) {
                    case EUp: {
                        COORD coords[8] = {{0,  0},
                                           {1,  -1},
                                           {2,  0},
                                           {3,  1},
                                           {2,  2},
                                           {1,  2},
                                           {0,  2},
                                           {-1, 2}};
                        memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
                        iterator->actionsSize = 8;
                        break;
                    }
                    case ERight: {
                        COORD coords[8] = {{1,  0},
                                           {2,  1},
                                           {1,  2},
                                           {0,  3},
                                           {-1, 2},
                                           {-1, 1},
                                           {-1, 0},
                                           {0,  1}};
                        memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
                        break;
                    }
                    case EDown: {
                        COORD coords[8] = {{0,  -1},
                                           {1,  -1},
                                           {2,  -1},
                                           {3,  0},
                                           {2,  1},
                                           {1,  2},
                                           {0,  1},
                                           {-1, 0}};
                        memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
                        break;
                    }
                    case ELeft: {
                        COORD coords[8] = {{0,  0},
                                           {1,  -1},
                                           {2,  0},
                                           {2,  1},
                                           {2,  2},
                                           {1,  3},
                                           {0,  2},
                                           {-1, 1}};
                        memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
                        break;
                    }
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return iterator;
}

VOID FreeIterator(HANDLE iterator) {
    HeapFree(GetProcessHeap(), 0, iterator);
}

HANDLE CopyFigure(HANDLE hFigure) {
    TetrisFigure* pCopy = malloc(sizeof(TetrisFigure));
    TetrisFigure* pInitFigure = (TetrisFigure*)hFigure;
    pCopy->coords = pInitFigure->coords;
    HANDLE hNewCoords = CreateVector(EStruct, sizeof(COORD));
    for(int i = 0; i < Size(pInitFigure->hCoordsList); i++) {
        COORD* pCoords = GetAt(pInitFigure->hCoordsList, i);
        AddElement(hNewCoords, pCoords, sizeof(COORD));
    }
    pCopy->hCoordsList = hNewCoords;
    pCopy->figureType = pInitFigure->figureType;
    pCopy->position = pInitFigure->position;

    return pCopy;
}

void FreeFigure(HANDLE hFigure) {
    TetrisFigure* pFigure = (TetrisFigure*)hFigure;
    if(pFigure != NULL) {
        DeleteList(pFigure->hCoordsList);
    }
    HeapFree(GetProcessHeap(), 0, pFigure);
}

DWORD GetCollisionBoundPixel(HANDLE hIterator, COORD *pPixelCoord) {
    PixelIterator *iterator = (PixelIterator *) hIterator;
    if (iterator == NULL) {
        return 0;
        return 0;
    }
    if (iterator->nCurrentAction == iterator->nActionsSize) {
        return TRAVERSE_END;
    }
    COORD action = ((COORD *) iterator->actionsArray)[iterator->nCurrentAction];
    TetrisFigure *pFigure = iterator->tetrisFigure;
    if (pFigure == NULL) {
        return 0;
    }
    *pPixelCoord = pFigure->coords;
    pPixelCoord->X += action.X;
    pPixelCoord->Y += action.Y;
    return HAS_NEXT;
}

void SetTetrisFigureCoordinates(HANDLE hFigure, COORD coords) {
    TetrisFigure *pFigure = (TetrisFigure *) hFigure;
    if (pFigure != NULL) {
        pFigure->coords.X = coords.X;
        pFigure->coords.Y = coords.Y;
    }
}

BOOL GetFigurePixels(DWORD dwPixelsList) {
    return FALSE;
}