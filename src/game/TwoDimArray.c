#include "TwoDimArray.h"
#include "malloc.h"
#include "memory.h"

HANDLE CreateTwoDimArray(size_t height, size_t width) {
    TwoDim* array = malloc(sizeof(TwoDim));
    array->data = malloc(height * width * sizeof(unsigned char));
    for(size_t i = 0; i < height * width; i++) {
        array->data[i] = 0;
    }
    array->width = width;
    array->height = height;
    return array;
}

void Set(HANDLE hArray, size_t x, size_t y, unsigned char value) {
    TwoDim* array = (TwoDim*)hArray;
    if(array != NULL) {
        if(x < array->width && y < array->height) {
            size_t flatIdx = y * array->width + x;
            array->data[flatIdx] = value;
        }
    }
}

unsigned char Get(HANDLE hArray, size_t x, size_t y) {
    TwoDim* array = (TwoDim*)hArray;
    if(array != NULL) {
        if(x < array->width && y < array->height) {
            size_t flatIdx = y * array->width + x;
            return array->data[flatIdx];
        }
    }
    return 0;
}

void RemoveLine(HANDLE hArray, size_t y) {
    TwoDim* array = (TwoDim*)hArray;
    if(array != NULL) {
        if(y < array->height) {
            memmove(array->data + array->width * sizeof(unsigned char),
                    array->data,
                    array->width * sizeof(unsigned char) * y);
        }
        memset(array->data, 0, array->width);
    }
}

void DestroyArray(HANDLE hArray) {
    TwoDim* array = (TwoDim*)hArray;
    if(array != NULL) {
        free(array->data);
        free(hArray);
    }
}