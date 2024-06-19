#pragma once
#include "StringLib.h"

struct _Stream {
    String buff;		/* Buffer for over-reads */
    void* parm;		/* The stream handle for core stream */
    int isEOF;			/* track file's EOF state */
    int maxBuffSz;
};

typedef struct _Stream* Stream;
typedef const struct _Stream* ConstStream;

/* Stream functions */
STRING_EXPORT Stream StreamOpen(String fileName);
STRING_EXPORT void* StreamClose(Stream stream);
STRING_EXPORT int StreamBuffLength(Stream stream, int size);
STRING_EXPORT int StreamReadLine(String string, Stream stream, char terminator);
STRING_EXPORT int StreamReadLines(String reader, Stream stream, ConstString term);
STRING_EXPORT int StreamRead(String reader, Stream stream, int n);
STRING_EXPORT int StreamReadLineAppend(String reader, Stream stream, char terminator);
STRING_EXPORT int StreamReadLinesAppend(String reader, Stream stream, ConstString term);
STRING_EXPORT int StreamReadAppend(String string, Stream stream, int n);
STRING_EXPORT int StreamUnread(Stream stream, ConstString terminator);
STRING_EXPORT int StreamPeek(String reader, ConstStream stream);
STRING_EXPORT int StreamEOF(ConstStream stream);
STRING_EXPORT String GetChar(Stream stream);