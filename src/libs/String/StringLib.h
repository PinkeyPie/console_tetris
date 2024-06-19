#pragma once

#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "types.h"
#include "StringGlobal.h"

#define STR_ERR (-1)
#define STR_OK (0)
#define STR_BS_BUFF_LENGTH_GET (0)

typedef struct _TagString* String;
typedef const struct _TagString* ConstString;

/* Copy functions */
STRING_EXPORT String FromCString(const char* str);
STRING_EXPORT String BytesToString(const void* bytes, int len);
STRING_EXPORT char* StringToCString(ConstString string);
STRING_EXPORT String StrCopy(ConstString string);

/* Destroy function */
STRING_EXPORT int Destroy(String string);

/* Space allocation hinting functions */
STRING_EXPORT int Alloc(String s, int len);
STRING_EXPORT String Substring(ConstString string, int left, int len);

/* Various standard manipulations */
STRING_EXPORT int Concat(String first, ConstString second);
STRING_EXPORT int ConcatCh(String string, char ch);
STRING_EXPORT int ConcatCString(String string, const char* str);
STRING_EXPORT int ConcatBytes(String string, const void* bytes, int len);
STRING_EXPORT int Insert(String string, int pos, ConstString substring, unsigned char fill);
STRING_EXPORT int InsertBytes(String string, int pos, const void* bytes, int len, unsigned char fill);
STRING_EXPORT int InsertCh(String string, int pos, int len, unsigned char fill);
STRING_EXPORT int Replace(String string, int pos, int len, ConstString replace);
STRING_EXPORT int Remove(String string, int pos, int len);
STRING_EXPORT int ReplaceString(String string, int pos, ConstString substring);
STRING_EXPORT int Truncate(String string, int n);

/* Scan/search functions */
STRING_EXPORT int StrCompareCaseless(ConstString first, ConstString second);
STRING_EXPORT int StrNCompareCaseless(ConstString first, ConstString second, int n);
STRING_EXPORT BOOL EqualsCaseless(ConstString first, ConstString second);
STRING_EXPORT BOOL EqualsCaselessBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT BOOL EqualsNCaselessBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT BOOL Equals(ConstString first, ConstString second);
STRING_EXPORT BOOL EqualsBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT BOOL EqualsNBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT BOOL EqualsCString(ConstString string, const char* cStr);
STRING_EXPORT BOOL EqualsCStrCaseless(ConstString string, const char* cStr);
STRING_EXPORT int StrCompare(ConstString first, ConstString second);
STRING_EXPORT int StrNCompare(ConstString first, ConstString second, int n);
STRING_EXPORT int Contains(ConstString string, int pos, ConstString substring);
STRING_EXPORT int ContainsCaseless(ConstString string, int pos, ConstString substring);
STRING_EXPORT int ContainsCh(ConstString string, int ch, int pos);
STRING_EXPORT int Search(ConstString string, int pos, ConstString substring);
STRING_EXPORT int SearchExclude(ConstString string, int pos, ConstString substring);
STRING_EXPORT int FindReplace(String string, ConstString find, ConstString repl, int pos);
STRING_EXPORT int FindReplaceCaseless(String string, ConstString find, ConstString repl, int pos);
STRING_EXPORT String Format(const char* fmt, ...);
STRING_EXPORT int FormatString(String string, const char* format, ...);

/* Miscellaneous functions */
STRING_EXPORT int ToUpper(String string);
STRING_EXPORT int ToLower(String string);
STRING_EXPORT int LTrim(String string);
STRING_EXPORT int RTrim(String string);
STRING_EXPORT int Trim(String string);

struct _TagString {
    int StrLen;
    unsigned char* Data;
};

/* Accessor macros */
#define LengthElse(String, Else)                (((String) == (void *)0 || (String)->StrLen < 0) ? (int)(Else) : ((String)->StrLen))
#define Length(String)                          (LengthElse ((String), 0))
#define DataOffsetElse(String, Offset, Else)    (((String) == (void *)0 || (String)->Data == (void*)0) ? (char *)(Else) : ((char *)(String)->Data) + (Offset))
#define DataOffset(String, Offset)              (DataOffsetElse ((String), (Offset), (void *)0))
#define DataElse(String, Else)                  (DataOffsetElse (String, 0, Else))
#define Data(String)                            (DataOffset (String, 0))
#define CharElse(String, Pos, Else)             ((((unsigned)(Pos)) < (unsigned)Length(String)) ? ((String)->Data[(Pos)]) : (Else))
#define Char(String, Pos)                       CharElse ((String), (Pos), '\0')