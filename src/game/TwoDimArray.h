#pragma once
#include "types.h"

typedef struct _TwoDim {
    unsigned char* data;
    size_t height;
    size_t width;
} TwoDim;

HANDLE CreateTwoDimArray(size_t height, size_t width);
void Set(HANDLE hArray, size_t x, size_t y, unsigned char value);
unsigned char Get(HANDLE hArray, size_t x, size_t y);
void RemoveLine(HANDLE hArray, size_t y);
void DestroyArray(HANDLE hArray);
