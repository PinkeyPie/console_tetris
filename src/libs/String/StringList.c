//#if defined (_MSC_VER)
//# define _CRT_SECURE_NO_WARNINGS
//#endif
//
//#include <stdio.h>
//#include <stddef.h>
//#include <stdarg.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <limits.h>
//#include "StringList.h"
//#include "StringLib.h"
//#include "Collection.h"
//
//struct genBstrList {
//    String b;
//    struct StringList* bl;
//};
//
///*  struct bstrList * bsplit (const_bstring str, unsigned char splitChar)
// *
// *  Create an array of sequential substrings from str divided by the character
// *  splitChar.
// */
//
//static int StringCB (void * parm, int ofs, int len) {
//    struct genBstrList * g = (struct genBstrList *) parm;
//    DWORD dwList = *((DWORD*)parm);
//    if(dwList != UNINITIALIZED_COLLECTION) {
//
//    }
//    if (g->bl->qty >= g->bl->mlen) {
//        int mlen = g->bl->mlen * 2;
//        String* tbl;
//
//        while (g->bl->qty >= mlen) {
//            if (mlen < g->bl->mlen) return BSTR_ERR;
//            mlen += mlen;
//        }
//
//        tbl = (String*)realloc(g->bl->entry, sizeof(String) * mlen);
//        if (tbl == NULL) return BSTR_ERR;
//
//        g->bl->entry = tbl;
//        g->bl->mlen = mlen;
//    }
//
//    g->bl->entry[g->bl->qty] = Substring(g->b, ofs, len);
//    g->bl->qty++;
//    return STR_OK;
//}
//
//DWORD Split(ConstString str, unsigned char splitChar) {
//    struct genBstrList g;
//
//    if (str == NULL || str->Data == NULL || str->StrLen < 0) {
//        return UNINITIALIZED_COLLECTION;
//    }
//
//    DWORD stringList = CreateList();
//    if(stringList == UNINITIALIZED_COLLECTION) {
//        return UNINITIALIZED_COLLECTION;
//    }
//
//    g.bl->mlen = 4;
//    g.bl->entry = (String*)malloc(g.bl->mlen * sizeof(String));
//    if (NULL == g.bl->entry) {
//        StrFree(g.bl);
//        return NULL;
//    }
//
//    g.b = (String)str;
//    g.bl->qty = 0;
//    if (SplitCb(str, splitChar, 0, StringCB, &g) < 0) {
//        StringListDestroy(g.bl);
//        return NULL;
//    }
//    return g.bl;
//}
//
///*  struct bstrList * bsplits (const_bstring str, bstring splitStr)
// *
// *  Create an array of sequential substrings from str divided by any of the
// *  characters in splitStr.  An empty splitStr causes a single entry bstrList
// *  containing a copy of str to be returned.
// */
//struct StringList* Splits(ConstString str, ConstString splitStr) {
//    struct genBstrList g;
//
//    if (str == NULL || str->StrLen < 0 || str->Data == NULL ||
//        splitStr == NULL || splitStr->StrLen < 0 || splitStr->Data == NULL)
//        return NULL;
//
//    g.bl = (struct StringList*)StrAlloc(sizeof(struct StringList));
//    if (g.bl == NULL) return NULL;
//    g.bl->MaxLen = 4;
//    g.bl->entry = (String*)StrAlloc(g.bl->MaxLen * sizeof(String));
//    if (NULL == g.bl->entry) {
//        StrFree(g.bl);
//        return NULL;
//    }
//    g.b = (String)str;
//    g.bl->qty = 0;
//
//    if (SplitsCb(str, splitStr, 0, StringCB, &g) < 0) {
//        StringListDestroy(g.bl);
//        return NULL;
//    }
//    return g.bl;
//}
//
///*  bstring bjoinblk (const struct bstrList * bl, void * blk, int len);
// *
// *  Join the entries of a bstrList into one bstring by sequentially
// *  concatenating them with the content from blk for length len in between.
// *  If there is an error NULL is returned, otherwise a bstring with the
// *  correct result is returned.
// */
//String JoinBytes(const struct StringList* bl, const void* s, int len) {
//    String b;
//    unsigned char* p;
//    int i, c, v;
//
//    if (bl == NULL || bl->qty < 0) return NULL;
//    if (len < 0) return NULL;
//    if (len > 0 && s == NULL) return NULL;
//    if (bl->qty < 1) return FromStatic("");
//
//    for (i = 0, c = 1; i < bl->qty; i++) {
//        v = bl->entry[i]->StrLen;
//        if (v < 0) return NULL;	/* Invalid input */
//        if (v > INT_MAX - c) return NULL;	/* Overflow */
//        c += v;
//    }
//
//    b = (String)StrAlloc(sizeof(struct _TagString));
//    if (len == 0) {
//        p = b->Data = (unsigned char*)StrAlloc(c);
//        if (p == NULL) {
//            StrFree(b);
//            return NULL;
//        }
//        for (i = 0; i < bl->qty; i++) {
//            v = bl->entry[i]->StrLen;
//            StrMemcpy(p, bl->entry[i]->Data, v);
//            p += v;
//        }
//    }
//    else {
//        v = (bl->qty - 1) * len;
//        if ((bl->qty > 512 || len > 127) &&
//            v / len != bl->qty - 1) return NULL; /* Overflow */
//        if (v > INT_MAX - c) return NULL;	/* Overflow */
//        c += v;
//        p = b->Data = (unsigned char*)StrAlloc(c);
//        if (p == NULL) {
//            StrFree(b);
//            return NULL;
//        }
//        v = bl->entry[0]->StrLen;
//        StrMemcpy(p, bl->entry[0]->Data, v);
//        p += v;
//        for (i = 1; i < bl->qty; i++) {
//            StrMemcpy(p, s, len);
//            p += len;
//            v = bl->entry[i]->StrLen;
//            if (v) {
//                StrMemcpy(p, bl->entry[i]->Data, v);
//                p += v;
//            }
//        }
//    }
//    b->MaxLen = c;
//    b->StrLen = c - 1;
//    b->Data[c - 1] = (unsigned char)'\0';
//    return b;
//}
//
///*  bstring bjoin (const struct bstrList * bl, const_bstring sep);
// *
// *  Join the entries of a bstrList into one bstring by sequentially
// *  concatenating them with the sep string in between.  If there is an error
// *  NULL is returned, otherwise a bstring with the correct result is returned.
// */
//String Join(const struct StringList* bl, ConstString sep) {
//    if (sep != NULL && (sep->StrLen < 0 || sep->Data == NULL)) return NULL;
//    return JoinBytes(bl, sep->Data, sep->StrLen);
//}
//
//#define BSSSC_BUFF_LEN (256)
//
///*  int bstrListCreate (void)
// *
// *  Create a bstrList.
// */
//struct StringList* CreateStringList(void) {
//    struct StringList* sl =
//            (struct StringList*)StrAlloc(sizeof(struct StringList));
//    if (sl) {
//        sl->entry = (String*)StrAlloc(1 * sizeof(String));
//        if (!sl->entry) {
//            StrFree(sl);
//            sl = NULL;
//        }
//        else {
//            sl->qty = 0;
//            sl->mlen = 1;
//        }
//    }
//    return sl;
//}
//
///*  int bstrListDestroy (struct bstrList * sl)
// *
// *  Destroy a bstrList that has been created by bsplit, bsplits or
// *  bstrListCreate.
// */
//int StringListDestroy(struct StringList* sl) {
//    int i;
//    if (sl == NULL || sl->qty < 0) return STR_ERR;
//    for (i = 0; i < sl->qty; i++) {
//        if (sl->entry[i]) {
//            Destroy(sl->entry[i]);
//            sl->entry[i] = NULL;
//        }
//    }
//    sl->qty = -1;
//    sl->mlen = -1;
//    StrFree(sl->entry);
//    sl->entry = NULL;
//    StrFree(sl);
//    return STR_OK;
//}
//
///*  int bstrListAlloc (struct bstrList * sl, int msz)
// *
// *  Ensure that there is memory for at least msz number of entries for the
// *  list.
// */
//int StringListAlloc(struct StringList* sl, int msz) {
//    String* l;
//    int smsz;
//    size_t nsz;
//    if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 ||
//        sl->qty > sl->mlen) return STR_ERR;
//    if (sl->mlen >= msz) return STR_OK;
//    smsz = SnapUpSize(msz);
//    nsz = ((size_t)smsz) * sizeof(String);
//    if (nsz < (size_t)smsz) return STR_ERR;
//    l = (String*)realloc(sl->entry, nsz);
//    if (!l) {
//        smsz = msz;
//        nsz = ((size_t)smsz) * sizeof(String);
//        l = (String*)realloc(sl->entry, nsz);
//        if (!l) return STR_ERR;
//    }
//    sl->mlen = smsz;
//    sl->entry = l;
//    return STR_OK;
//}
//
///*  int bstrListAllocMin (struct bstrList * sl, int msz)
// *
// *  Try to allocate the minimum amount of memory for the list to include at
// *  least msz entries or sl->qty whichever is greater.
// */
//int StringListAllocMin(struct StringList* sl, int msz) {
//    String* l;
//    size_t nsz;
//    if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 ||
//        sl->qty > sl->mlen) return STR_ERR;
//    if (msz < sl->qty) msz = sl->qty;
//    if (sl->mlen == msz) return STR_OK;
//    nsz = ((size_t)msz) * sizeof(String);
//    if (nsz < (size_t)msz) return STR_ERR;
//    l = (String*)realloc(sl->entry, nsz);
//    if (!l) return STR_ERR;
//    sl->mlen = msz;
//    sl->entry = l;
//    return STR_OK;
//}
//
///*  int bsplitcb (const_bstring str, unsigned char splitChar, int pos,
// *                int (* cb) (void * parm, int ofs, int len), void * parm)
// *
// *  Iterate the set of disjoint sequential substrings over str divided by the
// *  character in splitChar.
// *
// *  Note: Non-destructive modification of str from within the cb function
// *  while performing this split is not undefined.  bsplitcb behaves in
// *  sequential lock step with calls to cb.  I.e., after returning from a cb
// *  that return a non-negative integer, bsplitcb continues from the position
// *  1 character after the last detected split character and it will halt
// *  immediately if the length of str falls below this point.  However, if the
// *  cb function destroys str, then it *must* return with a negative value,
// *  otherwise bsplitcb will continue in an undefined manner.
// */
//int SplitCb(ConstString str, unsigned char splitChar, int pos, int (*cb) (void* parm, int ofs, int len), void* parm) {
//    int i, p, ret;
//
//    if (cb == NULL || str == NULL || pos < 0 || pos > str->StrLen) {
//        return STR_ERR;
//    }
//
//    p = pos;
//    do {
//        for (i = p; i < str->StrLen; i++) {
//            if (str->Data[i] == splitChar) {
//                break;
//            }
//        }
//        if ((ret = cb(parm, p, i - p)) < 0) {
//            return ret;
//        }
//        p = i + 1;
//    } while (p <= str->StrLen);
//    return STR_OK;
//}
//
///*  int bsplitscb (const_bstring str, const_bstring splitStr, int pos,
// *                 int (* cb) (void * parm, int ofs, int len), void * parm)
// *
// *  Iterate the set of disjoint sequential substrings over str divided by any
// *  of the characters in splitStr.  An empty splitStr causes the whole str to
// *  be iterated once.
// *
// *  Note: Non-destructive modification of str from within the cb function
// *  while performing this split is not undefined.  bsplitscb behaves in
// *  sequential lock step with calls to cb.  I.e., after returning from a cb
// *  that return a non-negative integer, bsplitscb continues from the position
// *  1 character after the last detected split character and it will halt
// *  immediately if the length of str falls below this point.  However, if the
// *  cb function destroys str, then it *must* return with a negative value,
// *  otherwise bsplitscb will continue in an undefined manner.
// */
//int SplitsCb(ConstString str, ConstString splitStr, int pos,
//             int (*cb) (void* parm, int ofs, int len), void* parm) {
//    struct charField chrs;
//    int i, p, ret;
//
//    if (cb == NULL || str == NULL || pos < 0 || pos > str->StrLen
//        || splitStr == NULL || splitStr->StrLen < 0) return STR_ERR;
//    if (splitStr->StrLen == 0) {
//        if ((ret = cb(parm, 0, str->StrLen)) > 0) ret = 0;
//        return ret;
//    }
//
//    if (splitStr->StrLen == 1)
//        return SplitCb(str, splitStr->Data[0], pos, cb, parm);
//
//    buildCharField(&chrs, splitStr);
//
//    p = pos;
//    do {
//        for (i = p; i < str->StrLen; i++) {
//            if (testInCharField(&chrs, str->Data[i])) break;
//        }
//        if ((ret = cb(parm, p, i - p)) < 0) return ret;
//        p = i + 1;
//    } while (p <= str->StrLen);
//    return STR_OK;
//}
//
///*  int bsplitstrcb (const_bstring str, const_bstring splitStr, int pos,
// *	int (* cb) (void * parm, int ofs, int len), void * parm)
// *
// *  Iterate the set of disjoint sequential substrings over str divided by the
// *  substring splitStr.  An empty splitStr causes the whole str to be
// *  iterated once.
// *
// *  Note: Non-destructive modification of str from within the cb function
// *  while performing this split is not undefined.  bsplitstrcb behaves in
// *  sequential lock step with calls to cb.  I.e., after returning from a cb
// *  that return a non-negative integer, bsplitscb continues from the position
// *  1 character after the last detected split character and it will halt
// *  immediately if the length of str falls below this point.  However, if the
// *  cb function destroys str, then it *must* return with a negative value,
// *  otherwise bsplitscb will continue in an undefined manner.
// */
//int SplitsStrCb(ConstString str, ConstString splitStr, int pos,
//                int (*cb) (void* parm, int ofs, int len), void* parm) {
//    int i, p, ret;
//
//    if (cb == NULL || str == NULL || pos < 0 || pos > str->StrLen
//        || splitStr == NULL || splitStr->StrLen < 0) return STR_ERR;
//
//    if (0 == splitStr->StrLen) {
//        for (i = pos; i < str->StrLen; i++) {
//            if ((ret = cb(parm, i, 1)) < 0) return ret;
//        }
//        return STR_OK;
//    }
//
//    if (splitStr->StrLen == 1)
//        return SplitCb(str, splitStr->Data[0], pos, cb, parm);
//
//    for (i = p = pos; i <= str->StrLen - splitStr->StrLen; i++) {
//        if (0 == memcmp(splitStr->Data, str->Data + i,
//                        splitStr->StrLen)) {
//            if ((ret = cb(parm, p, i - p)) < 0) return ret;
//            i += splitStr->StrLen;
//            p = i;
//        }
//    }
//    if ((ret = cb(parm, p, str->StrLen - p)) < 0) return ret;
//    return STR_OK;
//}
//
///*  struct bstrList * bsplitstr (const_bstring str, const_bstring splitStr)
// *
// *  Create an array of sequential substrings from str divided by the entire
// *  substring splitStr.
// */
//struct StringList* SplitStr(ConstString str, ConstString splitStr) {
//    struct genBstrList g;
//
//    if (str == NULL || str->Data == NULL || str->StrLen < 0) return NULL;
//
//    g.bl = (struct StringList*)StrAlloc(sizeof(struct StringList));
//    if (g.bl == NULL) return NULL;
//    g.bl->mlen = 4;
//    g.bl->entry = (String*)StrAlloc(g.bl->mlen * sizeof(String));
//    if (NULL == g.bl->entry) {
//        StrFree(g.bl);
//        return NULL;
//    }
//
//    g.b = (String)str;
//    g.bl->qty = 0;
//    if (SplitsStrCb(str, splitStr, 0, StringCB, &g) < 0) {
//        StringListDestroy(g.bl);
//        return NULL;
//    }
//    return g.bl;
//}
//
//
///* On IRIX vsnprintf returns n-1 when the operation would overflow the target
//   buffer, WATCOM and MSVC both return -1, while C99 requires that the
//   returned value be exactly what the length would be if the buffer would be
//   large enough.  This leads to the idea that if the return value is larger
//   than n, then changing n to the return value will reduce the number of
//   iterations required. */
