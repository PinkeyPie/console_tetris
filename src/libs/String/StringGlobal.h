//
// Created by PC on 07.06.2024.
//

#ifndef CONSOLE_TETRIS_STRINGGLOBAL_H
#define CONSOLE_TETRIS_STRINGGLOBAL_H
#ifdef STRING_LIBRARY
#ifdef _MSC_VER
#define STRING_EXPORT __declspec(dllexport)
#else
#define STRING_EXPORT
#endif
#else
#ifdef _MSC_VER
#define STRING_EXPORT __declspec(dllimport)
#else
#define STRING_EXPORT
#endif
#endif

#endif //CONSOLE_TETRIS_STRINGGLOBAL_H
