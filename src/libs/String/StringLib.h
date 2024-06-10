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
STRING_EXPORT String FromCStrAlloc(int MaxLen, const char* str);
STRING_EXPORT String FromCStrRangeAlloc(int minLen, int maxLen, const char* str);
STRING_EXPORT String BytesToString(const void* bytes, int len);
STRING_EXPORT char* StringToCStr(ConstString string, char ch);
STRING_EXPORT int CStringFree(char* string);
STRING_EXPORT String StrCopy(ConstString string);
STRING_EXPORT int Assign(String assigned, ConstString setString);
STRING_EXPORT int AssignMidStr(String assigned, ConstString setString, int left, int len);
STRING_EXPORT int AssignCStr(String assigned, const char* setString);
STRING_EXPORT int AssignBytes(String assigned, const void* bytes, int len);

/* Destroy function */
STRING_EXPORT int Destroy(String string);

/* Space allocation hinting functions */
STRING_EXPORT int Alloc(String s, int len);
STRING_EXPORT int AllocMin(String b, int len);
STRING_EXPORT String Substring(ConstString string, int left, int len);

/* Various standard manipulations */
STRING_EXPORT int Concat(String first, ConstString second);
STRING_EXPORT int ConcatCh(String string, char ch);
STRING_EXPORT int ConcatCStr(String string, const char* str);
STRING_EXPORT int ConcatBytes(String string, const void* bytes, int len);
STRING_EXPORT int Insert(String string, int pos, ConstString substring, unsigned char fill);
STRING_EXPORT int InsertBytes(String string, int pos, const void* bytes, int len, unsigned char fill);
STRING_EXPORT int InsertCh(String string, int pos, int len, unsigned char fill);
STRING_EXPORT int Replace(String string, int pos, int len, ConstString replace, unsigned char fill);
STRING_EXPORT int Remove(String string, int pos, int len);
STRING_EXPORT int ReplaceString(String string, int pos, ConstString substring, unsigned char fill);
STRING_EXPORT int Truncate(String string, int n);

/* Scan/search functions */
STRING_EXPORT int StrCompareCaseless(ConstString first, ConstString second);
STRING_EXPORT int StrNCompareCaseless(ConstString first, ConstString second, int n);
STRING_EXPORT int EqualsCaseless(ConstString first, ConstString second);
STRING_EXPORT int EqualsCaselessBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT int StemEqualsCaseLessBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT int Equals(ConstString first, ConstString second);
STRING_EXPORT int EqualsBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT int StemEqualsBytes(ConstString string, const void* bytes, int len);
STRING_EXPORT int EqualsCStr(ConstString string, const char* cStr);
STRING_EXPORT int EqualsCStrCaseless(ConstString string, const char* cStr);
STRING_EXPORT int StrCompare(ConstString first, ConstString second);
STRING_EXPORT int StrNCompare(ConstString first, ConstString second, int n);
STRING_EXPORT int Contains(ConstString string, int pos, ConstString substring);
STRING_EXPORT int ContainsReverse(ConstString string, int pos, ConstString substring);
STRING_EXPORT int ContainsCaseless(ConstString string, int pos, ConstString substring);
STRING_EXPORT int ContainsReverseCaseless(ConstString string, int pos, ConstString substring);
STRING_EXPORT int ContainsCh(ConstString string, int ch, int pos);
STRING_EXPORT int ContainsChReverse(ConstString string, int ch, int pos);
STRING_EXPORT int Search(ConstString string, int pos, ConstString substring);
STRING_EXPORT int SearchReverse(ConstString string, int pos, ConstString substring);
STRING_EXPORT int SearchExclude(ConstString string, int pos, ConstString substring);
STRING_EXPORT int SearchExcludeReverse(ConstString string, int pos, ConstString substring);
STRING_EXPORT int FindReplace(String string, ConstString find, ConstString repl, int pos);
STRING_EXPORT int FindReplaceCaseless(String string, ConstString find, ConstString repl, int pos);
STRING_EXPORT String Format(const char* fmt, ...);
STRING_EXPORT int FormatString(String string, const char* format, ...);
STRING_EXPORT int AssignFormat(String string, const char* fmt, ...);
STRING_EXPORT int VcFormat(String string, int count, const char* fmt, va_list arglist);

/* Miscellaneous functions */
STRING_EXPORT int Pattern(String string, int len);
STRING_EXPORT int ToUpper(String string);
STRING_EXPORT int ToLower(String string);
STRING_EXPORT int LTrim(String string);
STRING_EXPORT int RTrim(String string);
STRING_EXPORT int Trim(String string);

struct _TagString {
    int MaxLen;
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

/* Static constant string initialization macro */
#define StaticStringMaxLen(CString,MaxLen)      {(m), (int) sizeof(q)-1, (unsigned char *) ("" q "")}
#if defined(_MSC_VER)
# define StaticString(CString)           StaticStringMaxLen(CString,-32)
#endif
#ifndef StaticString
# define StaticString(CString)           StaticStringMaxLen(CString,-__LINE__)
#endif

/* Static constant block parameter pair */
#define StaticStringByte(CString) ((void *)("" CString "")), ((int) sizeof(CString)-1)

#define ConcatStatic(bytes,str)         ((ConcatBytes)((bytes), StaticStringByte(str)))
#define FromStatic(str)                 ((BytesToString)(StaticStringByte(str)))
#define AssignStatic(bytes,str)         ((AssignBytes)((bytes), StaticStringByte(str)))
#define InsertStatic(b,p,s,f)           ((InsertBytes)((b), (p), StaticStringByte(s), (f)))
#define JoinStatic(b,s)                 ((JoinBulk)((b), StaticStringByte(s)))
#define EqualsStatic(b,s)               ((EqualsBytes)((b), StaticStringByte(s)))
#define StemEqualsStatic(b,s)           ((StemEqualsBytes)((b), StaticStringByte(s)))
#define EqualsCaselessStatic(b,s)       ((EqualsCaseLessBlk)((b), StaticStringByte(s)))
#define StemEqualsCaselessStatic(b,s)   ((StemEqualsCaseLessBlk)((b), StaticStringByte(s)))

/* Reference building macros */
#define StringFromCStr(t,s) {                                               \
    (t).data = (unsigned char *) (s);                                       \
    (t).slen = ((t).data) ? ((int) (strlen) ((char *)(t).data)) : 0;        \
    (t).mlen = -1;                                                          \
}
#define StringFromBytes(t,s,l) {               \
    (t).Data = (unsigned char *) (s);       \
    (t).StrLen = l;                           \
    (t).MaxLen = -1;                          \
}