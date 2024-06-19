#pragma once
#include "StringLib.h"

extern HANDLE CreateStringList(void);
extern int StringListDestroy(HANDLE stringList);
/* String split and join functions */
extern HANDLE Split(ConstString str, unsigned char splitChar);
extern HANDLE SplitByString(ConstString str, ConstString splitStr);
extern String Join(HANDLE stringList, ConstString sep);
extern String JoinBytes(HANDLE stringList, const void* bytes, int len);