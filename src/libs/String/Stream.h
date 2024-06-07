#pragma once
#include "String.h"

struct Stream {
    String buff;		/* Buffer for over-reads */
    void* parm;		/* The stream handle for core stream */
    bNread readFnPtr;	/* fread compatible fnptr for core stream */
    int isEOF;			/* track file's EOF state */
    int maxBuffSz;
};

/* Stream functions */
extern struct Stream* StreamOpen(bNread readPtr, void* parm);
extern void* StreamClose(struct Stream* s);
extern int StreamBuffLength(struct Stream* s, int sz);
extern int StreamReadLine(String b, struct Stream* s, char terminator);
extern int StreamReadLines(String r, struct Stream* s, ConstString term);
extern int StreamRead(String b, struct Stream* s, int n);
extern int StreamReadLineA(String b, struct Stream* s, char terminator);
extern int StreamReadLinesA(String r, struct Stream* s, ConstString term);
extern int StreamReadA(String b, struct Stream* s, int n);
extern int StreamUnread(struct Stream* s, ConstString b);
extern int StreamPeek(String r, const struct Stream* s);
extern int StreamSplitsCb(struct Stream* s, ConstString splitStr, int (*cb) (void* parm, int ofs, ConstString entry), void* parm);
extern int StreamSplitsStringCb(struct Stream* s, ConstString splitStr, int (*cb) (void* parm, int ofs, ConstString entry), void* parm);
extern int StreamEOF(const struct Stream* s);