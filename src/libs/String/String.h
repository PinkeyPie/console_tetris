#pragma once

#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define STR_ERR (-1)
#define STR_OK (0)
#define STR_BS_BUFF_LENGTH_GET (0)

typedef struct _TagString* String;
typedef const struct _TagString* ConstString;

/* Copy functions */
extern String FromCStr(const char* str);
extern String FromCStrAlloc(int mlen, const char* str);
extern String FromCStrRangeAlloc(int minl, int maxl, const char* str);
extern String BytesToString(const void* blk, int len);
extern char* StringToCStr(ConstString string, char ch);
extern int CStrFree(char* string);
extern String StrCpy(ConstString b1);
extern int Assign(String a, ConstString b);
extern int AssignMidStr(String a, ConstString b, int left, int len);
extern int AssignCStr(String a, const char* str);
extern int AssignBytes(String a, const void* s, int len);

/* Destroy function */
extern int Destroy(String b);

/* Space allocation hinting functions */
extern int Alloc(String s, int len);
extern int AllocMin(String b, int len);
extern String Substring(ConstString b, int left, int len);

/* Various standard manipulations */
extern int Concat(String b0, ConstString b1);
extern int ConcatCh(String b0, char c);
extern int ConcatCStr(String b, const char* s);
extern int ConcatBytes(String b, const void* s, int len);
extern int Insert(String s1, int pos, ConstString s2, unsigned char fill);
extern int InsertBytes(String s1, int pos, const void* s2, int len, unsigned char fill);
extern int InsertCh(String s1, int pos, int len, unsigned char fill);
extern int Replace(String b1, int pos, int len, ConstString b2, unsigned char fill);
extern int Remove(String s1, int pos, int len);
extern int SetString(String b0, int pos, ConstString b1, unsigned char fill);
extern int Trunc(String b, int n);

/* Scan/search functions */
extern int StrComp(ConstString b0, ConstString b1);
extern int StrnComp(ConstString b0, ConstString b1, int n);
extern int EqualsCaseLess(ConstString b0, ConstString b1);
extern int EqualsCaseLessBlk(ConstString b, const void* blk, int len);
extern int StemEqualsCaseLessBlk(ConstString b0, const void* blk, int len);
extern int Equals(ConstString b0, ConstString b1);
extern int EqualsBytes(ConstString b, const void* blk, int len);
extern int StemEqualsBytes(ConstString b0, const void* blk, int len);
extern int EqualsCStr(ConstString b, const char* s);
extern int EqualsCStringCaseLess(ConstString b, const char* s);
extern int bstrcmp(ConstString b0, ConstString b1);
extern int bstrncmp(ConstString b0, ConstString b1, int n);
extern int Contains(ConstString s1, int pos, ConstString s2);
extern int ContainsReverse(ConstString s1, int pos, ConstString s2);
extern int ContainsCaseLess(ConstString s1, int pos, ConstString s2);
extern int binstrrcaseless(ConstString s1, int pos, ConstString s2);
extern int ContainsCh(ConstString b, int c, int pos);
extern int ContainsChReverse(ConstString b, int c, int pos);
extern int Search(ConstString b0, int pos, ConstString b1);
extern int SearchReverse(ConstString b0, int pos, ConstString b1);
extern int SearchExclude(ConstString b0, int pos, ConstString b1);
extern int SearchExcludeReverse(ConstString b0, int pos, ConstString b1);
extern int FindReplace(String b, ConstString find, ConstString repl, int pos);
extern int FindReplaceCaseLess(String b, ConstString find, ConstString repl, int pos);
extern String Format(const char* fmt, ...);
extern int FormatA(String b, const char* fmt, ...);
extern int AssignFormat(String b, const char* fmt, ...);
extern int VcFormat(String b, int count, const char* fmt, va_list arglist);

#define bvformata(ret, b, fmt, lastarg) { \
String bstrtmp_b = (b); \
const char * bstrtmp_fmt = (fmt); \
int bstrtmp_r = STR_ERR, bstrtmp_sz = 16; \
	for (;;) { \
		va_list bstrtmp_arglist; \
		va_start (bstrtmp_arglist, lastarg); \
		bstrtmp_r = VcFormat (bstrtmp_b, bstrtmp_sz, bstrtmp_fmt, bstrtmp_arglist); \
		va_end (bstrtmp_arglist); \
		if (bstrtmp_r >= 0) { /* Everything went ok */ \
			bstrtmp_r = STR_OK; \
			break; \
		} else if (-bstrtmp_r <= bstrtmp_sz) { /* A real error? */ \
			bstrtmp_r = STR_ERR; \
			break; \
		} \
		bstrtmp_sz = -bstrtmp_r; /* Doubled or target size */ \
	} \
	ret = bstrtmp_r; \
}

typedef int (*bNgetc) (void* parm);
typedef size_t(*bNread) (void* buff, size_t elsize, size_t nelem, void* parm);

/* Input functions */
extern String GetC(bNgetc getcPtr, void* parm, char terminator);
extern String Read(bNread readPtr, void* parm);
extern int GetSa(String b, bNgetc getcPtr, void* parm, char terminator);
extern int AssignGetStr(String b, bNgetc getcPtr, void* parm, char terminator);
extern int ReadA(String b, bNread readPtr, void* parm);

struct _TagString {
    int mlen;
    int slen;
    unsigned char* data;
};

/* Accessor macros */
#define LengthEx(b, e)          (((b) == (void *)0 || (b)->slen < 0) ? (int)(e) : ((b)->slen))
#define Length(b)               (LengthEx ((b), 0))
#define DataOfStrEx(b, o, e)    (((b) == (void *)0 || (b)->data == (void*)0) ? (char *)(e) : ((char *)(b)->data) + (o))
#define DataOfStr(b, o)         (DataOfStrEx ((b), (o), (void *)0))
#define DataEx(b, e)            (DataOfStrEx (b, 0, e))
#define Data(b)                 (DataOfStr (b, 0))
#define CharEx(b, p, e)         ((((unsigned)(p)) < (unsigned)Length(b)) ? ((b)->data[(p)]) : (e))
#define Char(b, p)              CharEx ((b), (p), '\0')

/* Static constant string initialization macro */
#define StrStaticMLen(q,m)      {(m), (int) sizeof(q)-1, (unsigned char *) ("" q "")}
#if defined(_MSC_VER)
# define StrStatic(q)           StrStaticMLen(q,-32)
#endif
#ifndef StrStatic
# define StrStatic(q)           StrStaticMLen(q,-__LINE__)
#endif

/* Static constant block parameter pair */
#define StrStaticBlkParms(q) ((void *)("" q "")), ((int) sizeof(q)-1)

#define CatStatic(b,s)                  ((ConcatBytes)((b), StrStaticBlkParms(s)))
#define FromStatic(s)                   ((BytesToString)(StrStaticBlkParms(s)))
#define AssignStatic(b,s)               ((AssignBytes)((b), StrStaticBlkParms(s)))
#define InsertStatic(b,p,s,f)           ((InsertBytes)((b), (p), StrStaticBlkParms(s), (f)))
#define JoinStatic(b,s)                 ((JoinBulk)((b), StrStaticBlkParms(s)))
#define EqualsStatic(b,s)               ((EqualsBytes)((b), StrStaticBlkParms(s)))
#define StemEqualsStatic(b,s)           ((StemEqualsBytes)((b), StrStaticBlkParms(s)))
#define EqualsCaseLessStatic(b,s)       ((EqualsCaseLessBlk)((b), StrStaticBlkParms(s)))
#define StemEqualsCaseLessStatic(b,s)   ((StemEqualsCaseLessBlk)((b), StrStaticBlkParms(s)))

/* Reference building macros */
#define CStrToString btfromcstr
#define StringFromCStr(t,s) {                                               \
    (t).data = (unsigned char *) (s);                                       \
    (t).slen = ((t).data) ? ((int) (strlen) ((char *)(t).data)) : 0;        \
    (t).mlen = -1;                                                          \
}
#define BulkToString(t,s,l) {               \
    (t).data = (unsigned char *) (s);       \
    (t).slen = l;                           \
    (t).mlen = -1;                          \
}
#define btfromblk(t,s,l) BulkToString(t,s,l)
#define bmid2tbstr(t,b,p,l) {                                                \
    ConstString bstrtmp_s = (b);                                           \
    if (bstrtmp_s && bstrtmp_s->data && bstrtmp_s->slen >= 0) {              \
        int bstrtmp_left = (p);                                              \
        int bstrtmp_len  = (l);                                              \
        if (bstrtmp_left < 0) {                                              \
            bstrtmp_len += bstrtmp_left;                                     \
            bstrtmp_left = 0;                                                \
        }                                                                    \
        if (bstrtmp_len > bstrtmp_s->slen - bstrtmp_left)                    \
            bstrtmp_len = bstrtmp_s->slen - bstrtmp_left;                    \
        if (bstrtmp_len <= 0) {                                              \
            (t).data = (unsigned char *)"";                                  \
            (t).slen = 0;                                                    \
        } else {                                                             \
            (t).data = bstrtmp_s->data + bstrtmp_left;                       \
            (t).slen = bstrtmp_len;                                          \
        }                                                                    \
    } else {                                                                 \
        (t).data = (unsigned char *)"";                                      \
        (t).slen = 0;                                                        \
    }                                                                        \
    (t).mlen = -__LINE__;                                                    \
}
#define btfromblkltrimws(t,s,l) {                                            \
    int bstrtmp_idx = 0, bstrtmp_len = (l);                                  \
    unsigned char * bstrtmp_s = (s);                                         \
    if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
        for (; bstrtmp_idx < bstrtmp_len; bstrtmp_idx++) {                   \
            if (!isspace (bstrtmp_s[bstrtmp_idx])) break;                    \
        }                                                                    \
    }                                                                        \
    (t).data = bstrtmp_s + bstrtmp_idx;                                      \
    (t).slen = bstrtmp_len - bstrtmp_idx;                                    \
    (t).mlen = -__LINE__;                                                    \
}
#define btfromblkrtrimws(t,s,l) {                                            \
    int bstrtmp_len = (l) - 1;                                               \
    unsigned char * bstrtmp_s = (s);                                         \
    if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
        for (; bstrtmp_len >= 0; bstrtmp_len--) {                            \
            if (!isspace (bstrtmp_s[bstrtmp_len])) break;                    \
        }                                                                    \
    }                                                                        \
    (t).data = bstrtmp_s;                                                    \
    (t).slen = bstrtmp_len + 1;                                              \
    (t).mlen = -__LINE__;                                                    \
}
#define btfromblktrimws(t,s,l) {                                             \
    int bstrtmp_idx = 0, bstrtmp_len = (l) - 1;                              \
    unsigned char * bstrtmp_s = (s);                                         \
    if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
        for (; bstrtmp_idx <= bstrtmp_len; bstrtmp_idx++) {                  \
            if (!isspace (bstrtmp_s[bstrtmp_idx])) break;                    \
        }                                                                    \
        for (; bstrtmp_len >= bstrtmp_idx; bstrtmp_len--) {                  \
            if (!isspace (bstrtmp_s[bstrtmp_len])) break;                    \
        }                                                                    \
    }                                                                        \
    (t).data = bstrtmp_s + bstrtmp_idx;                                      \
    (t).slen = bstrtmp_len + 1 - bstrtmp_idx;                                \
    (t).mlen = -__LINE__;                                                    \
}

/* Write protection macros */
#define bwriteprotect(t)     { if ((t).mlen >=  0) (t).mlen = -1; }
#define bwriteallow(t)       { if ((t).mlen == -1) (t).mlen = (t).slen + ((t).slen == 0); }
#define biswriteprotected(t) ((t).mlen <= 0)