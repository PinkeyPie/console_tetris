#include "Draw.h"
#ifdef WIN32
#include "windows.h"
#include "string.h"
#endif

#ifdef WIN32
void GotoXY(short x, short y) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = {x, y};
    SetConsoleCursorPosition(hStdOut, position);
}

void DrawGameWindow() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    short nCols, nRows;
    GetConsoleScreenBufferInfo(hStdOut, &bufferInfo);
    nCols = (short)(bufferInfo.srWindow.Left + 1);
    nRows = (short)(bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1);

    COORD dwSize = {1, 1};
    for(short i = 0; i < nRows; i++) {
        if(i == 0 || i == nRows - 1) {
            COORD dwCoords = {0, i};
            DWORD dwStrSize = nCols * sizeof(TCHAR);
            PTCHAR szOutput = HeapAlloc(GetProcessHeap(), 0, dwStrSize);
            wsprintf(szOutput, "");
            WriteConsoleOutput(hStdOut, "+", dwSize, dwCoords, NULL);
//            printf_s("+");
            for(short j = 1; j < nCols - 1; j++) {
                dwCoords.X = j;
                dwCoords.Y = i;
                WriteConsoleOutput(hStdOut, "+", dwSize, dwCoords, NULL);
//                printf_s("-");
            }
            dwCoords.X = (short)(nCols - 1);
            dwCoords.Y = i;
            WriteConsoleOutput(hStdOut, "+", dwSize, dwCoords, NULL);
//            printf_s("+");
        } else {
            COORD dwCoords = {0, i};
            WriteConsoleOutput(hStdOut, "+", dwSize, dwCoords, NULL);
//            printf_s("|");
            dwCoords.X = (short)(nCols - 1);
            dwCoords.Y = i;
            WriteConsoleOutput(hStdOut, "+", dwSize, dwCoords, NULL);
//            printf_s("|");
        }
    }
}
#endif