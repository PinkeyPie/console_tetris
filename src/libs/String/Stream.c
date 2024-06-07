#pragma once
#include "String.h"

struct StringList {
    int qty, mlen;
    String* entry;
};

extern struct StringList* CreateStringList(void);
extern int StringListDestroy(struct StringList* sl);
extern int StringListAlloc(struct StringList* sl, int msz);
extern int StringListAllocMin(struct StringList* sl, int msz);

/* String split and join functions */
extern struct StringList* Split(ConstString str, unsigned char splitChar);
extern struct StringList* Splits(ConstString str, ConstString splitStr);
extern struct StringList* SplitStr(ConstString str, ConstString splitStr);
extern String Join(const struct StringList* bl, ConstString sep);
extern String JoinBulk(const struct StringList* bl, const void* s, int len);
extern int SplitCb(ConstString str, unsigned char splitChar, int pos, int (*cb) (void* parm, int ofs, int len), void* parm);
extern int SplitsCb(ConstString str, ConstString splitStr, int pos, int (*cb) (void* parm, int ofs, int len), void* parm);
extern int SplitsStrCb(ConstString str, ConstString splitStr, int pos, int (*cb) (void* parm, int ofs, int len), void* parm);

/* Miscellaneous functions */
extern int Pattern(String b, int len);
extern int ToUpper(String b);
extern int ToLower(String b);
extern int LTrim(String b);
extern int RTrim(String b);
extern int Trim(String b);