#pragma once

typedef unsigned long long DWORD;
typedef unsigned char BOOL;

#define TRUE 1
#define FALSE 0

typedef enum EType {
    String,
    Int,
    Long,
    Float,
    Double,
    Struct,
    Unknown
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