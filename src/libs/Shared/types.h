#pragma once
#include "wchar.h"

#ifndef WIN32
typedef unsigned long long DWORD;
typedef unsigned char BOOL;
typedef void* HANDLE;
typedef struct _COORD {
    short X;
    short Y;
} COORD;
typedef struct _SMALL_RECT {
    short Left;
    short Top;
    short Right;
    short Bottom;
} SMALL_RECT, *PSMALL_RECT;
#endif

#define TRUE 1
#define FALSE 0

typedef enum EType {
    EString,
    EInt,
    ELong,
    EFloat,
    EDouble,
    EStruct,
    EUnknown
} EType;

typedef enum ECollection {
    ELinkedList = 1 << 30,
    EVector = 2 << 30,
    EMap = 3 << 30
} ECollection;

typedef enum EErrorCode {
    EWrongCollection = 1
} EErrorCode;

typedef struct Error {
    EErrorCode code;
    wchar_t* szMessage;
} Error;

typedef void(*Destructor)(void*);