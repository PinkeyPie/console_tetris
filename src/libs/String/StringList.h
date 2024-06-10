#pragma once
//#include "StringLib.h"
//
//extern DWORD CreateStringList(void);
//extern int StringListDestroy(DWORD stringList);
//extern int StringListAlloc(DWORD stringList, int maxSize);
//extern int StringListAllocMin(DWORD stringList, int maxSize);
//
///* String split and join functions */
//extern DWORD Split(ConstString str, unsigned char splitChar);
//extern DWORD Splits(ConstString str, ConstString splitStr);
//extern DWORD SplitStr(ConstString str, ConstString splitStr);
//extern String Join(DWORD stringList, ConstString sep);
//extern String JoinBytes(DWORD stringList, const void* bytes, int len);
//extern int SplitCb(ConstString str, unsigned char splitChar, int pos, int (*cb) (void* parm, int ofs, int len), void* parm);
//extern int SplitsCb(ConstString str, ConstString splitStr, int pos, int (*cb) (void* parm, int ofs, int len), void* parm);
//extern int SplitsStrCb(ConstString str, ConstString splitStr, int pos, int (*cb) (void* parm, int ofs, int len), void* parm);