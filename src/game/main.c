#include <stdio.h>
#ifdef WIN32
#include "windows.h"
#include "conio.h"
#endif
#include "StringLib.h"

int main() {
    String string = FromCString("Hello world");
    String replaceString = FromCString("Hola");
    String findPattern = FromCString("world");
    Replace(string, 0, 5, findPattern);
    int found = Search(string, 0, replaceString);
//    Replace(string, 0, 5, replaceString, ' ');
    Destroy(string);
    printf_s("Hello world");
    return 0;
}