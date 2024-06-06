#include "Draw.h"
#include "Collection.h"
#include "TetrisFigure.h"
#ifdef WIN32
#include "windows.h"
#include "string.h"
#include <process.h>
#endif

#ifdef WIN32
#define esc 27
#define FRAME_LONG 1000 / 60
#define MAX(first, second) first > second ? first : second
#define MIN(first, second) first < second ? first : second
static HANDLE ghMutex;
static short nCols = 0;
static short nRows = 0;
static short nLabelsVerticalMarginTop = 0;
static short nLabelsVerticalMarginBottom = 0;

static COORD menuLeftUp = { 0, 0 };

void GotoXY(short, short);
void ShowConsoleCursor();
void HideCursor();
void ClearConsole();
void ClearConsoleLine(COORD coord, DWORD nSize);
void WriteConsoleLine(COORD coord, PTCHAR szLine);
void GetUserKey(int* nStateKey);

typedef struct Container {
    COORD coords;
    COORD sizes;
} Container;

typedef struct MenuLabel {
    DWORD nLabelSize;
    PCHAR szLabel;
    COORD labelCoord;
    BOOL bSelected;
    BOOL bNeedsClear;
} MenuLabel;

void DestroyMenuLabel(void* pLabelMem) {
    if (pLabelMem != NULL) {
        MenuLabel* pMenuLabel = (MenuLabel*)pLabelMem;
        HeapFree(GetProcessHeap(), 0, pMenuLabel->szLabel);
        pMenuLabel->szLabel = NULL;
    }
}

static DWORD dwLabelsList = 0;
static DWORD dwGraphicalConveor = 0;
static DWORD dwNextFigures = 0;

static Container fieldContainer;
static Container nextItemContainer;
static Container scoresContainer;
static Container timeContainer;

static int nLastUserKey = -1;

void GotoXY(short x, short y) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { x, y };
    SetConsoleCursorPosition(hStdOut, position);
}

void ShowConsoleCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);
}

void HideCursor() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hStdOut, &info);
}

void ClearConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = csbi.dwSize.X;
    scrollRect.Bottom = csbi.dwSize.Y;

    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes;

    ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);
    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y = 0;

    SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
}

void GetConsoleBounds() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    GetConsoleScreenBufferInfo(hStdOut, &bufferInfo);
    nCols = (short)(bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1);
    nRows = (short)(bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1);
}

void ClearConsoleLine(COORD coord, DWORD nSize) {
    HANDLE hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hStdHandle, coord);
    PTCHAR szWhiteSpace = HeapAlloc(GetProcessHeap(), 0, nSize * sizeof(TCHAR) + 1);
    memset(szWhiteSpace, ' ', nSize);
    szWhiteSpace[nSize] = '\0';
    WriteConsole(hStdHandle, szWhiteSpace, nSize, NULL, NULL);
    HeapFree(GetProcessHeap(), 0, szWhiteSpace);
}

void WriteConsoleLine(COORD coord, PTCHAR szLine) {
    HANDLE hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hStdHandle, coord);
    DWORD nStringSize = strlen(szLine);
    WriteConsole(hStdHandle, szLine, nStringSize, NULL, 0);
}

void WriteConsoleChar(COORD coord, CHAR ch) {
    HANDLE hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hStdHandle, coord);
    
}

void RenderThread(void* pArg) {
    int nCurrentUserKey = 0;
    while (TRUE) {
        if (nLastUserKey == esc) {
            break;
        }
        LARGE_INTEGER frequency;
        LARGE_INTEGER start;
        LARGE_INTEGER end;
        int interval;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);
        GetUserKey(&nCurrentUserKey);


        QueryPerformanceCounter(&end);
        interval = ((double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000);
        Sleep(MAX(FRAME_LONG - interval, 0));
    }
    _endthread();
}

void AddMenuLabel(COORD coord, PCHAR szLabel, BOOL bSelected) {
    MenuLabel label;
    DWORD nLineSize = strlen(szLabel) + 1;
    label.szLabel = HeapAlloc(GetProcessHeap(), 0, nLineSize * sizeof(CHAR));
    strcpy_s(label.szLabel, nLineSize, szLabel);
    label.bSelected = bSelected;
    label.labelCoord = coord;
    label.nLabelSize = nLineSize;
    AddElement(dwLabelsList, &label, sizeof(MenuLabel));
}

void PrintCurrentMenu() {
    for (size_t i = 0; i < Size(dwLabelsList); i++) {
        MenuLabel* pLabel = (MenuLabel*)GetAt(dwLabelsList, i);
        COORD labelCoord = pLabel->labelCoord;
        if (labelCoord.X == 0 && labelCoord.Y == 0) {
            labelCoord.X = menuLeftUp.X;
            labelCoord.Y = menuLeftUp.Y + i * nLabelsVerticalMarginTop;
        }
        PCHAR szPrintLine = HeapAlloc(GetProcessHeap(), 0, (pLabel->nLabelSize + (pLabel->bSelected ?  3 : 0)) * sizeof(CHAR));
        if (pLabel->bSelected) {
            labelCoord.X -= 3;
            wsprintf(szPrintLine, "-> %s", pLabel->szLabel);
        }
        else {
            strcpy_s(szPrintLine, pLabel->nLabelSize, pLabel->szLabel);
        }
        WriteConsoleLine(labelCoord, szPrintLine);
        HeapFree(GetProcessHeap(), 0, szPrintLine);
    }
}

void ClearCurrentMenu() {
    for (size_t i = 0; i < Size(dwLabelsList); i++) {
        MenuLabel* pLabel = (MenuLabel*)GetAt(dwLabelsList, i);
        if (!pLabel->bNeedsClear) {
            continue;
        }
        COORD labelCoord = pLabel->labelCoord;
        if (labelCoord.X == 0 && labelCoord.Y == 0) {
            labelCoord.X = menuLeftUp.X;
            labelCoord.Y = menuLeftUp.Y + i * nLabelsVerticalMarginTop;
        }
        PCHAR szWhiteSpaceLine = HeapAlloc(GetProcessHeap(), 0, (pLabel->nLabelSize + (pLabel->bSelected ? 3 : 0)) * sizeof(CHAR));
        if (pLabel->bSelected) {
            labelCoord.X -= 3;
            memset(szWhiteSpaceLine, ' ', pLabel->nLabelSize + 3);
        }
        else {
            memset(szWhiteSpaceLine, ' ', pLabel->nLabelSize);
        }
        WriteConsoleLine(labelCoord, szWhiteSpaceLine);
        HeapFree(GetProcessHeap(), 0, szWhiteSpaceLine);
    }
}

void DrawBox(Container container) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GotoXY(container.coords.X, container.coords.Y);
    WriteConsole(hStdOut, "+", 1, NULL, NULL);
    GotoXY(container.coords.X + container.sizes.X, container.coords.Y);
    WriteConsole(hStdOut, "+", 1, NULL, NULL);
    GotoXY(container.coords.X, container.coords.Y + container.sizes.Y);
    WriteConsole(hStdOut, "+", 1, NULL, NULL);
    GotoXY(container.coords.X + container.sizes.X, container.coords.Y + container.sizes.Y);
    WriteConsole(hStdOut, "+", 1, NULL, NULL);
    for (short i = 1; i < container.sizes.Y; i++) {
        GotoXY(container.coords.X, container.coords.Y + i);
        WriteConsole(hStdOut, "|", 1, NULL, NULL);
        GotoXY(container.coords.X + container.sizes.X, container.coords.Y + i);
        WriteConsole(hStdOut, "|", 1, NULL, NULL);
    }
    for (short i = 1; i < container.sizes.X; i++) {
        GotoXY(container.coords.X + i, container.coords.Y);
        WriteConsole(hStdOut, "-", 1, NULL, NULL);
        GotoXY(container.coords.X + i, container.coords.Y + container.sizes.Y);
        WriteConsole(hStdOut, "-", 1, NULL, NULL);
    }
}

void ClearContainerData(Container container) {
    for (short i = 1; i < container.sizes.Y; i++) {
        COORD coords = { container.coords.X + 1, container.coords.Y + i };
        PCHAR szWhiteSpaces = HeapAlloc(GetProcessHeap(), 0, container.sizes.X - 1);
        szWhiteSpaces[container.sizes.X - 1] = '\0';
        memset(szWhiteSpaces, ' ', container.sizes.X - 2);
        WriteConsoleLine(coords, szWhiteSpaces);
    }
}

void DrawTetrisScreen() {
    fieldContainer.coords.X = 10;
    fieldContainer.coords.Y = 0;
    fieldContainer.sizes.X = 10;
    fieldContainer.sizes.Y = 20;

    nextItemContainer.coords.X = fieldContainer.coords.X + fieldContainer.sizes.X + 2;
    nextItemContainer.coords.Y = fieldContainer.coords.Y;
    nextItemContainer.sizes.X = 6;
    nextItemContainer.sizes.Y = fieldContainer.sizes.Y;

    timeContainer.sizes.Y = 2;
    timeContainer.coords.X = 1;
    timeContainer.coords.X = fieldContainer.coords.X - timeContainer.coords.X;
    timeContainer.coords.Y = (fieldContainer.coords.Y + fieldContainer.sizes.Y) - timeContainer.sizes.Y;

    scoresContainer.coords.X = 1;
    scoresContainer.sizes.Y = 2;
    scoresContainer.sizes.X = fieldContainer.coords.X - scoresContainer.coords.X;
    scoresContainer.coords.Y = timeContainer.coords.Y - scoresContainer.sizes.Y - 1;

    DrawBox(fieldContainer);
    DrawBox(nextItemContainer);
    DrawBox(scoresContainer);
    DrawBox(timeContainer);
}


void DrawTetrisFigure(TetrisFigure* figure) {
    /*
    COORD coords = figure->coords;
    switch (figure->figureType)
    {
    case TFigure:
        switch (figure->position)
        {
        case EUp:
            CHAR szLine[4];
            wsprintf(szLine, "***");
            coords.X -= 1;
            WriteConsoleLine(coords, szLine);
            wsprintf(szLine, " * ");
            coords.Y -= 1;
            WriteConsoleLine(coords, szLine);
            break;
        case ELeft:
            CHAR szLine[3];
            wsprintf(szLine, "* ");
            coords.X -= 1;
            coords.Y += 1;
            WriteConsoleLine(coords, szLine);
            wsprintf(szLine, "**");
            coords.Y -= 1;
            WriteConsoleLine(coords, szLine);
            wsprintf(szLine, "* ");
            coords.Y -= 1;
            WriteConsoleLine(coords, szLine);
            break;
        case EDown:
            CHAR szLine[4];
            wsprintf(szLine, "***");
            coords.X -= 1;
            WriteConsoleLine(coords, szLine);
            wsprintf(szLine, " * ");
            coords.Y += 1;
            WriteConsoleLine(coords, szLine);
            break;
        case ERight:
            CHAR szLine[3];
            wsprintf(szLine, " *");
            coords.X -= 1;
            coords.Y += 1;
            WriteConsoleLine(coords, szLine);
            wsprintf(szLine, "**");
            coords.Y -= 1;
            WriteConsoleLine(coords, szLine);
            wsprintf(szLine, " *");
            coords.Y -= 1;
            WriteConsoleLine(coords, szLine);
            break;
        default:
            break;
        }
        break;
    case LFigure:
        switch (figure->position)
        {
        case EUp:
            coords.X = 
            CHAR szLine[3];
            wsprintf(szLine, "* ");
            WriteConsoleLine()
        default:
            break;
        }
        break;
    case ZFigure:
        break;
    case RectFigure:
        break;
    case Stick:
        break;
    case ZReversed:
        break;
    case LReversed:
        break;
    default:
        break;
    }
    */
}

void DrawBlockLined(COORD leftUp, PCHAR szLinedBlock) {
    PCHAR szLine = strtok(szLinedBlock, "\n");
    while (szLine != NULL)
    {
        DWORD nLineSize = strlen(szLine);
        COORD coord = leftUp;
        for (int i = 0; i < nLineSize; i++) {
            if (szLine[i] == ' ') {
                coord.X++;
                continue;
            }
            if (szLine[i] == '\n') {
                break;
            }
            WriteConsoleLine(coord, "*");
        }
        szLine = strtok(NULL, "\n");
        leftUp.Y++;
    }
}

void ClearBlock(COORD leftUp, COORD sizes) {
    COORD coords = leftUp;
    for (int i = leftUp.Y; i < leftUp.Y + sizes.Y; i++) {
        PCHAR szWhiteSpaces = malloc(sizes.X + 1);
        memset(szWhiteSpaces, ' ', sizes.X);
        WriteConsoleLine(leftUp, szWhiteSpaces);
        coords.Y++;
    }
}

void DrawBlockMatrix(COORD leftUp, PCHAR* szMatrixBlock, size_t nLines) {
    for (int i = 0; i < nLines; i++) {
        WriteConsoleLine(leftUp, szMatrixBlock[i]);
        leftUp.Y++;
    }
}

void DrawTetrisNextFigures() {

}

void DrawGameWindow() {
    HideCursor();
    GetConsoleBounds();
    COORD lineCoords = { 0, 0 };
    menuLeftUp.X = (nCols / 3);
    menuLeftUp.Y = (nRows / 3);
    PCHAR szLine = HeapAlloc(GetProcessHeap(), 0, nCols + 1);
    szLine[nCols] = '\0';
    for (short i = 0; i < nRows; i++) {
        memset(szLine, ' ', nCols);
        if (i == 0 || i == nRows - 1) {
            szLine[0] = '+';
            szLine[nCols - 1] = '+';
            for (int j = 1; j < nCols - 1; j++) {
                szLine[j] = '-';
            }
        }
        else {
            szLine[0] = '|';
            szLine[nCols - 1] = '|';
        }
        WriteConsoleLine(lineCoords, szLine);
        lineCoords.Y++;
    }
    dwLabelsList = CreateList();
    dwGraphicalConveor = CreateList();
    SetDestroyFunction(dwLabelsList, &DestroyMenuLabel);
    lineCoords.Y = 0;
    wsprintf(szLine, "1. Start Game");
    AddMenuLabel(lineCoords, szLine, TRUE);
    wsprintf(szLine, "2. Game scores");
    AddMenuLabel(lineCoords, szLine, FALSE);
    wsprintf(szLine, "3. Exit game");
    AddMenuLabel(lineCoords, szLine, FALSE);
    PrintCurrentMenu();
    //ghMutex = CreateMutex(NULL, FALSE, NULL);
    //_beginthread(RenderThread, 0, NULL);
}

void GetUserKey(int* nStateKey) {
    WaitForSingleObject(ghMutex, INFINITE);
    *nStateKey = nLastUserKey;
    nLastUserKey = -1;
    ReleaseMutex(ghMutex);
}

void SetUserKey(int nKey) {
    WaitForSingleObject(ghMutex, INFINITE);
    nLastUserKey = nKey;
    ReleaseMutex(ghMutex);
}

#endif