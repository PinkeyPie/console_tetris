#include <stdio.h>
#include "Collection.h"
#include "stdlib.h"
#ifdef WIN32
#include "windows.h"
#include "conio.h"
#endif
#include "Draw.h"
#include "StringLib.h"
#include "Stream.h"

int main() {

    String string = FromCString("Hello world");
    String someLow = FromCString("     hello world     ");
    Trim(someLow);
    Concat(string, someLow);
    String copy = StrCopy(string);
    if(StrCompareCaseless(string, someLow)) {
        Truncate(string, 10);
    }

    (string, )int strlen = (int)sizeof("hello world")-1;
    int maxlen = (-32);
    unsigned char* data = (unsigned char*)("" "hello world" "");

    //DrawGameWindow();
//    DWORD dwLines = CreateVector(EString, sizeof(wchar_t));
//    AddStringElement(dwLines, L"Hello world");
//    wchar_t* szLine = GetStringAt(dwLines, 0);
    FreeCollections();
    return 0;
}