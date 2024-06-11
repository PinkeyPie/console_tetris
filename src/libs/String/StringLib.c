#if defined (_MSC_VER)
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "StringLib.h"
#include "types.h"

static int SnapUpSize(int i) {
    if (i < 8) {
        i = 8;
    }
    else {
        unsigned int j;
        j = (unsigned int)i;

        j |= (j >> 1);
        j |= (j >> 2);
        j |= (j >> 4);
        j |= (j >> 8);		// Ok, since int >= 16 bits
#if (UINT_MAX != 0xffff)
        j |= (j >> 16);		// For 32 bit int systems
#elif (UINT_MAX > 0xffffffffUL)
        j |= (j >> 32);		// For 64 bit int systems
#endif
        // Least power of two greater than i
        j++;
        if ((int)j >= i) i = (int)j;
    }
    return i;
}

// int Alloc (String string, int length)
// Increase the size of the memory backing the string to at least length.
int Alloc(String string, int length) {
    int len;
    if (string == NULL || string->Data == NULL || string->StrLen < 0 || string->MaxLen <= 0 ||
        string->MaxLen < string->StrLen || length <= 0) {
        return STR_ERR;
    }

    if (length >= string->MaxLen) {
        unsigned char* x;
        if ((len = SnapUpSize(length)) <= string->MaxLen) {
            return STR_OK;
        }
        // Assume probability of a non-moving realloc is 0.125
        if (7 * string->MaxLen < 8 * string->StrLen) {

            // If StrLen is close to MaxLen in size then use realloc to reduce
            // the memory defragmentation
            reallocStrategy:
            x = (unsigned char*)realloc(string->Data, (size_t)len);
            if (x == NULL) {
                // Since we failed, try allocating the tighest possible
                // allocation
                len = length;
                x = (unsigned char*)realloc(string->Data, (size_t)length);
                if (NULL == x) {
                    return STR_ERR;
                }
            }
        }
        else {

            // If StrLen is not close to MaxLen then avoid the penalty of copying
            // the extra bytes that are allocated, but not considered part of
            // the string

            if (NULL == (x = (unsigned char*)malloc((size_t)len))) {

                // Perhaps there is no available memory for the two
                // allocations to be in memory at once
                goto reallocStrategy;
            }
            else {
                if (string->StrLen) {
                    memcpy((char*)x, (char*)string->Data, (size_t)string->StrLen);
                }
                free(string->Data);
            }
        }
        string->Data = x;
        string->MaxLen = len;
        string->Data[string->StrLen] = (unsigned char)'\0';
    }

    return STR_OK;
}

/*  int ballocmin (bstring b, int len)
 *
 *  SetAt the size of the memory backing the bstring b to len or b->StrLen+1,
 *  whichever is larger.  Note that repeated use of this function can degrade
 *  performance.
 */
int AllocMin(String string, int length) {
    unsigned char* s;

    if (string == NULL || string->Data == NULL) return STR_ERR;
    if (string->StrLen >= INT_MAX || string->StrLen < 0) return STR_ERR;
    if (string->MaxLen <= 0 || string->MaxLen < string->StrLen || length <= 0) {
        return STR_ERR;
    }

    if (length < string->StrLen + 1) length = string->StrLen + 1;

    if (length != string->MaxLen) {
        s = (unsigned char*)realloc(string->Data, (size_t)length);
        if (NULL == s) return STR_ERR;
        s[string->StrLen] = (unsigned char)'\0';
        string->Data = s;
        string->MaxLen = length;
    }

    return STR_OK;
}

/*  String FromCString(const char* str)
 *
 *  Create a string which contains the contents of the '\0' terminated char *
 *  buffer str.
 */
String FromCString(const char* str) {
    String string;
    int i;
    size_t j;

    if (str == NULL) {
        return NULL;
    }
    j = strlen(str);
    i = SnapUpSize((int)(j + (2 - (j != 0))));
    if (i <= (int)j) {
        return NULL;
    }

    string = (String)malloc(sizeof(struct _TagString));
    if (string == NULL) {
        return NULL;
    }
    string->StrLen = (int)j;
    string->MaxLen = i;
    string->Data = (unsigned char*) malloc(string->MaxLen);
    if (string->Data == NULL) {
        free(string);
        return NULL;
    }

    memcpy(string->Data, str, j + 1);
    return string;
}

/*  bstring bfromcstrrangealloc (int minLen, int maxLen, const char* str)
 *
 *  Create a bstring which contains the contents of the '\0' terminated
 *  char* buffer str.  The memory buffer backing the string is at least
 *  minLen characters in length, but an attempt is made to allocate up to
 *  maxLen characters.
 */
String FromCStrRangeAlloc(int minLen, int maxLen, const char* str) {
    String string;
    int i;
    size_t j;

    /* Bad parameters? */
    if (str == NULL) {
        return NULL;
    }
    if (maxLen < minLen || minLen < 0) {
        return NULL;
    }

    /* Adjust lengths */
    j = strlen(str);
    if ((size_t)minLen < (j + 1)) {
        minLen = (int)(j + 1);
    }
    if (maxLen < minLen) {
        maxLen = minLen;
    }
    i = maxLen;

    string = (String)malloc(sizeof(struct _TagString));
    if (string == NULL) {
        return NULL;
    }
    string->StrLen = (int)j;
    string->MaxLen = i;
    string->Data = (unsigned char*)malloc(string->MaxLen);
    while (string->Data == NULL) {
        int k = (i >> 1) + (minLen >> 1);
        if (i == k || i < minLen) {
            free(string);
            return NULL;
        }
        i = k;
    }
    memcpy(string->Data, str, j + 1);
    return string;
}

/*  String FromCStrAlloc(int MaxLen, const char* str)
 *
 *  Create a string which contains the contents of the '\0' terminated
 *  char* buffer str.  The memory buffer backing the string is at least
 *  MaxLen characters in length.
 */
String FromCStrAlloc(int MaxLen, const char* str) {
    return FromCStrRangeAlloc(MaxLen, MaxLen, str);
}

/*  String BytesToString(const void* bytes, int len)
 *
 *  Create a string which contains the content of the block bytes of length
 *  len.
 */
String BytesToString(const void* bytes, int len) {
    String string;
    int i;

    if (bytes == NULL || len < 0) {
        return NULL;
    }
    string = (String)malloc(sizeof(struct _TagString));
    if (string == NULL) {
        return NULL;
    }
    string->StrLen = len;

    i = len + (2 - (len != 0));
    i = SnapUpSize(i);

    string->MaxLen = i;

    string->Data = (unsigned char*)malloc((size_t)string->MaxLen);
    if (string->Data == NULL) {
        free(string);
        return NULL;
    }

    if (len > 0) {
        memcpy(string->Data, bytes, (size_t)len);
    }
    string->Data[len] = (unsigned char)'\0';

    return string;
}

/*  char* StringToCStr(ConstString string, char ch)
 *
 *  Create a '\0' terminated char * buffer which is equal to the contents of
 *  the string s, except that any contained '\0' characters are converted
 *  to the character in ch. This returned value should be freed with a
 *  CStringFree () call, by the calling application.
 */
char* StringToCStr(ConstString string, char ch) {
    int i, l;
    char* result;

    if (string == NULL || string->StrLen < 0 || string->Data == NULL) {
        return NULL;
    }
    l = string->StrLen;
    result = (char*)malloc((size_t)(l + 1));
    if (result == NULL) {
        return result;
    }

    for (i = 0; i < l; i++) {
        result[i] = (char)((string->Data[i] == '\0') ? ch : (char)(string->Data[i]));
    }

    result[l] = (unsigned char)'\0';

    return result;
}

/*  int CStringFree(char* string)
 *
 *  Frees a C-string generated by StrToCStr ().  This is normally unnecessary
 *  since it just wraps a call to free (), however, if malloc ()
 *  and free () have been redefined as a macros within the String
 *  module (via defining them in memdbg.h)
 *  with some difference in behaviour from the std
 *  library functions, then this allows a correct way of freeing the memory
 *  that allows higher level code to be independent of these macro
 *  redefinitions.
 */
int CStringFree(char* string) {
    if (string) {
        free(string);
        return STR_OK;
    }
    return STR_ERR;
}

/*  int Concat(String first, ConstString second)
 *
 *  Concatenate the String second to the String first.
 */
int Concat(String first, ConstString second) {
    int len, d;
    String aux = (String)second;

    if (first == NULL || second == NULL || first->Data == NULL || second->Data == NULL) {
        return STR_ERR;
    }

    d = first->StrLen;
    len = second->StrLen;
    if ((d | (first->MaxLen - d) | len | (d + len)) < 0) {
        return STR_ERR;
    }

    if (first->MaxLen <= d + len + 1) {
        ptrdiff_t pd = second->Data - first->Data;
        if (0 <= pd && pd < first->MaxLen) {
            if (NULL == (aux = StrCopy(second))) return STR_ERR;
        }
        if (Alloc(first, d + len + 1) != STR_OK) {
            if (aux != second) Destroy(aux);
            return STR_ERR;
        }
    }

    if(len > 0) {
        memmove(&first->Data[d], &aux->Data[0], (size_t) len);
    }
    first->Data[d + len] = (unsigned char)'\0';
    first->StrLen = d + len;
    if (aux != second) {
        Destroy(aux);
    }
    return STR_OK;
}

/*  int ConcatCh(String string, char ch)
 *
 *  Concatenate the single character ch to the String string.
 */
int ConcatCh(String string, char ch) {
    int dst;

    if (string == NULL) {
        return STR_ERR;
    }
    dst = string->StrLen;
    if ((dst | (string->MaxLen - dst)) < 0 || Alloc(string, dst + 2) != STR_OK)
        return STR_ERR;
    string->Data[dst] = (unsigned char)ch;
    string->Data[dst + 1] = (unsigned char)'\0';
    string->StrLen++;
    return STR_OK;
}

/*  int ConcatCStr(String string, const char* str)
 *
 *  Concatenate a char * string to a String.
 */
int ConcatCStr(String string, const char* str) {
    char* dest;
    int i, len;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 || string->MaxLen < string->StrLen
        || string->MaxLen <= 0 || str == NULL) {
        return STR_ERR;
    }

    /* Optimistically concatenate directly */
    len = string->MaxLen - string->StrLen;
    dest = (char*)&string->Data[string->StrLen];
    for (i = 0; i < len; i++) {
        if ((*dest++ = *str++) == '\0') {
            string->StrLen += i;
            return STR_OK;
        }
    }
    string->StrLen += i;

    /* Need to explicitely resize and concatenate tail */
    return ConcatBytes(string, (const void*)str, (int)strlen(str));
}

/*  int ConcatBytes(String string, const void* bytes, int len)
 *
 *  Concatenate a fixed length buffer to a bstring.
 */
int ConcatBytes(String string, const void* bytes, int len) {
    int newLen;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 || string->MaxLen < string->StrLen
        || string->MaxLen <= 0 || bytes == NULL || len < 0) {
        return STR_ERR;
    }

    if ((newLen = string->StrLen + len) < 0) {
        return STR_ERR; /* Overflow? */
    }
    if (string->MaxLen <= newLen && Alloc(string, newLen + 1) < 0) {
        return STR_ERR;
    }

    if(len > 0) {
        memmove(&string->Data[string->StrLen], bytes, (size_t) len);
    }
    string->StrLen = newLen;
    string->Data[newLen] = (unsigned char)'\0';
    return STR_OK;
}

/*  String StrCopy(ConstString string)
 *
 *  Create a copy of the String string.
 */
String StrCopy(ConstString string) {
    String copyStr;
    int i, j;

    /* Attempted to copy an invalid string? */
    if (string == NULL || string->StrLen < 0 || string->Data == NULL) {
        return NULL;
    }

    copyStr = (String)malloc(sizeof(struct _TagString));
    if (copyStr == NULL) {
        /* Unable to allocate memory for string header */
        return NULL;
    }

    i = string->StrLen;
    j = SnapUpSize(i + 1);

    copyStr->Data = (unsigned char*)malloc(j);
    if (copyStr->Data == NULL) {
        j = i + 1;
        copyStr->Data = (unsigned char*)malloc(j);
        if (copyStr->Data == NULL) {
            /* Unable to allocate memory for string Data */
            free(copyStr);
            return NULL;
        }
    }

    copyStr->MaxLen = j;
    copyStr->StrLen = i;

    if (i) {
        memcpy((char*)copyStr->Data, (char*)string->Data, i);
    }
    copyStr->Data[copyStr->StrLen] = (unsigned char)'\0';

    return copyStr;
}

/*  int Assign(String assigned, ConstString setString)
 *
 *  Overwrite the string assigned with the contents of string setString.
 */
int Assign(String assigned, ConstString setString) {
    if (setString == NULL || setString->Data == NULL || setString->StrLen < 0) {
        return STR_ERR;
    }
    if (setString->StrLen != 0) {
        if (Alloc(assigned, setString->StrLen) != STR_OK) {
            return STR_ERR;
        }
        memmove (assigned->Data, setString->Data, setString->StrLen);
    }
    else {
        if (assigned == NULL || assigned->Data == NULL || assigned->MaxLen < assigned->StrLen ||
            assigned->StrLen < 0 || assigned->MaxLen == 0) {
            return STR_ERR;
        }
    }
    assigned->Data[setString->StrLen] = (unsigned char)'\0';
    assigned->StrLen = setString->StrLen;
    return STR_OK;
}

/*  int AssignMidStr(String assigned, ConstString setString, int left, int len)
 *
 *  Overwrite the string assigned with the middle of contents of string setString
 *  starting from position left and running for assigned length len.  left and
 *  len are clamped to the ends of setString as with the function MidStr.
 */
int AssignMidStr(String assigned, ConstString setString, int left, int len) {
    if (setString == NULL || setString->Data == NULL || setString->StrLen < 0) {
        return STR_ERR;
    }

    if (left < 0) {
        len += left;
        left = 0;
    }

    if (len > setString->StrLen - left) {
        len = setString->StrLen - left;
    }

    if (assigned == NULL || assigned->Data == NULL || assigned->MaxLen < assigned->StrLen ||
        assigned->StrLen < 0 || assigned->MaxLen == 0) {
        return STR_ERR;
    }

    if (len > 0) {
        if (Alloc(assigned, len) != STR_OK) {
            return STR_ERR;
        }
        memmove (assigned->Data, setString->Data + left, len);
        assigned->StrLen = len;
    }
    else {
        assigned->StrLen = 0;
    }
    assigned->Data[assigned->StrLen] = (unsigned char)'\0';
    return STR_OK;
}

/*  int AssignCStr(String assigned, const char* setString)
 *
 *  Overwrite the string assigned with the contents of char * string setString.  Note that
 *  the string assigned must be assigned well-defined and writable string.  If an error
 *  occurs STR_ERR is returned however assigned may be partially overwritten.
 */
int AssignCStr(String assigned, const char* setString) {
    int i;
    size_t len;
    if (assigned == NULL || assigned->Data == NULL || assigned->MaxLen < assigned->StrLen ||
        assigned->StrLen < 0 || assigned->MaxLen == 0 || NULL == setString) {
        return STR_ERR;
    }

    for (i = 0; i < assigned->MaxLen; i++) {
        if ('\0' == (assigned->Data[i] = setString[i])) {
            assigned->StrLen = i;
            return STR_OK;
        }
    }

    assigned->StrLen = i;
    len = strlen(setString + i);
    if (len + 1 > (size_t) INT_MAX - i || Alloc(assigned, (int)(i + len + 1)) < 0) {
        return STR_ERR;
    }
    if(len + 1) {
        memmove(assigned->Data + i, setString + i, (size_t) len + 1);
    }
    assigned->StrLen += (int)len;
    return STR_OK;
}

/*  int AssignBytes(String assigned, const void* bytes, int len)
 *
 *  Overwrite the string assigned with the contents of the block (bytes, len).  Note that
 *  the string assigned must be assigned well-defined and writable string.  If an error
 *  occurs STR_ERR is returned and assigned is not overwritten.
 */
int AssignBytes(String assigned, const void* bytes, int len) {
    if (assigned == NULL || assigned->Data == NULL || assigned->MaxLen < assigned->StrLen ||
        assigned->StrLen < 0 || assigned->MaxLen == 0 || NULL == bytes || len < 0 || len >= INT_MAX) {
        return STR_ERR;
    }
    if (len + 1 > assigned->MaxLen && 0 > Alloc(assigned, len + 1)) {
        return STR_ERR;
    }
    if((size_t)len > 0) {
        memmove(assigned->Data, bytes, (size_t) len);
    }
    assigned->Data[len] = (unsigned char)'\0';
    assigned->StrLen = len;
    return STR_OK;
}

/*  int btrunc (bstring string, int n)
 *
 *  Truncate the bstring to at most n characters.
 */
int Truncate(String string, int n) {
    if (n < 0 || string == NULL || string->Data == NULL || string->MaxLen < string->StrLen ||
        string->StrLen < 0 || string->MaxLen <= 0) {
        return STR_ERR;
    }
    if (string->StrLen > n) {
        string->StrLen = n;
        string->Data[n] = (unsigned char)'\0';
    }
    return STR_OK;
}

/*  int ToUpper(String string)
 *
 *  Convert contents of string to upper case.
 */
int ToUpper(String string) {
    int i, len;
    if (string == NULL || string->Data == NULL || string->MaxLen < string->StrLen ||
        string->StrLen < 0 || string->MaxLen <= 0) {
        return STR_ERR;
    }
    for (i = 0, len = string->StrLen; i < len; i++) {
        string->Data[i] = (unsigned char)toupper(string->Data[i]);
    }
    return STR_OK;
}

/*  int ToLower(String string)
 *
 *  Convert contents of string to lower case.
 */
int ToLower(String string) {
    int i, len;
    if (string == NULL || string->Data == NULL || string->MaxLen < string->StrLen ||
        string->StrLen < 0 || string->MaxLen <= 0) {
        return STR_ERR;
    }
    for (i = 0, len = string->StrLen; i < len; i++) {
        string->Data[i] = (unsigned char)tolower(string->Data[i]);
    }
    return STR_OK;
}

/*  int bstricmp (const_bstring first, const_bstring second)
 *
 *  Compare two strings without differentiating between case.  The return
 *  value is the difference of the values of the characters where the two
 *  strings first differ after lower case transformation, otherwise 0 is
 *  returned indicating that the strings are equal.  If the lengths are
 *  different, then a difference from 0 is given, but if the first extra
 *  character is '\0', then it is taken to be the value UCHAR_MAX+1.
 */
int StrCompareCaseless(ConstString first, ConstString second) {
    int i, result, n;

    if (Data(first) == NULL || first->StrLen < 0 ||
        Data(second) == NULL || second->StrLen < 0) {
        return SHRT_MIN;
    }
    if ((n = first->StrLen) > second->StrLen) {
        n = second->StrLen;
    } else if (first->StrLen == second->StrLen && first->Data == second->Data) {
        return STR_OK;
    }

    for (i = 0; i < n; i++) {
        result = (char)tolower(first->Data[i]) - (char)tolower(second->Data[i]);
        if (result != 0) {
            return result;
        }
    }

    if (first->StrLen > n) {
        result = (char)tolower(first->Data[n]);
        if (result) {
            return result;
        }
        return UCHAR_MAX + 1;
    }
    if (second->StrLen > n) {
        result = -(char)tolower(second->Data[n]);
        if (result) {
            return result;
        }
        return -(int)(UCHAR_MAX + 1);
    }
    return STR_OK;
}

/*  int StrNCompareCaseless(ConstString first, ConstString second, int n)
 *
 *
 *  Compare two strings without differentiating between case for at most n
 *  characters.  If the position where the two strings first differ is
 *  before the nth position, the return value is the difference of the values
 *  of the characters, otherwise 0 is returned.  If the lengths are different
 *  and less than n characters, then a difference from 0 is given, but if the
 *  first extra character is '\0', then it is taken to be the value
 *  UCHAR_MAX+1.
 */
int StrNCompareCaseless(ConstString first, ConstString second, int n) {
    int i, v, m;

    if (Data(first) == NULL || first->StrLen < 0 || Data(second) == NULL || second->StrLen < 0 || n < 0) {
        return SHRT_MIN;
    }
    m = n;
    if (m > first->StrLen) {
        m = first->StrLen;
    }
    if (m > second->StrLen) {
        m = second->StrLen;
    }

    if (first->Data != second->Data) {
        for (i = 0; i < m; i++) {
            v = (char)tolower(first->Data[i]);
            v -= (char)tolower(second->Data[i]);
            if (v != 0) {
                return first->Data[i] - second->Data[i];
            }
        }
    }

    if (n == m || first->StrLen == second->StrLen) {
        return STR_OK;
    }

    if (first->StrLen > m) {
        v = (char)tolower(first->Data[m]);
        if (v) {
            return v;
        }
        return UCHAR_MAX + 1;
    }

    v = -(char)tolower(second->Data[m]);
    if (v) {
        return v;
    }
    return -(int)(UCHAR_MAX + 1);
    return -(int)(UCHAR_MAX + 1);
}

/*  int EqualsCaselessBytes(ConstString string, const void* bytes, int len)
 *
 *  Compare content of string and the array of bytes in bytes for length len for
 *  equality without differentiating between character case.  If the content
 *  differs other than in case, 0 is returned, if, ignoring case, the content
 *  is the same, 1 is returned, if there is an error, -1 is returned.  If the
 *  length of the strings are different, this function is O(1).  '\0'
 *  characters are not treated in any special way.
 */
int EqualsCaselessBytes(ConstString string, const void* bytes, int len) {
    int i;

    if (Data(string) == NULL || string->StrLen < 0 || bytes == NULL || len < 0) {
        return STR_ERR;
    }
    if (string->StrLen != len) {
        return 0;
    }
    if (len == 0 || string->Data == bytes) {
        return 1;
    }
    for (i = 0; i < len; i++) {
        if (string->Data[i] != ((unsigned char*)bytes)[i]) {
            unsigned char c = (unsigned char)tolower(string->Data[i]);
            if (c != (unsigned char)tolower(((unsigned char*)bytes)[i])) {
                return 0;
            }
        }
    }
    return 1;
}


/*  int EqualsCaseless(ConstString first, ConstString second)
 *
 *  Compare two strings for equality without differentiating between case.
 *  If the strings differ other than in case, 0 is returned, if the strings
 *  are the same, 1 is returned, if there is an error, -1 is returned.  If
 *  the length of the strings are different, this function is O(1).  '\0'
 *  termination characters are not treated in any special way.
 */
int EqualsCaseless(ConstString first, ConstString second) {
    if (NULL == second) {
        return STR_ERR;
    }
    return EqualsCaselessBytes(first, second->Data, second->StrLen);
}

/*  int StemEqualsCaseLessBytes(ConstString string, const void* bytes, int len)
 *
 *  Compare beginning of string with a block of memory of length len
 *  without differentiating between case for equality.  If the beginning of string
 *  differs from the memory block other than in case (or if string is too short),
 *  0 is returned, if the strings are the same, 1 is returned, if there is an
 *  error, -1 is returned.  '\0' characters are not treated in any special
 *  way.
 */
int StemEqualsCaseLessBytes(ConstString string, const void* bytes, int len) {
    int i;

    if (Data(string) == NULL || string->StrLen < 0 || NULL == bytes || len < 0) {
        return STR_ERR;
    }
    if (string->StrLen < len) {
        return STR_OK;
    }
    if (string->Data == (const unsigned char*)bytes || len == 0) {
        return 1;
    }

    for (i = 0; i < len; i++) {
        if (string->Data[i] != ((const unsigned char*)bytes)[i]) {
            if (tolower(string->Data[i]) != tolower(((const unsigned char*)bytes)[i])) {
                return 0;
            }
        }
    }
    return 1;
}

/*
 * int LTrim(String string)
 *
 * Delete whitespace contiguous from the left end of the string.
 */
int LTrim(String string) {
    int i, len;

    if (string == NULL || string->Data == NULL ||
    string->MaxLen < string->StrLen || string->StrLen < 0 || string->MaxLen <= 0) {
        return STR_ERR;
    }

    for (len = string->StrLen, i = 0; i < len; i++) {
        if (!isspace(string->Data[i])) {
            return Remove(string, 0, i);
        }
    }

    string->Data[0] = (unsigned char)'\0';
    string->StrLen = 0;
    return STR_OK;
}

/*
 * int RTrim(String string)
 *
 * Delete whitespace contiguous from the right end of the string.
 */
int RTrim(String string) {
    int i;

    if (string == NULL || string->Data == NULL || string->MaxLen < string->StrLen ||
        string->StrLen < 0 || string->MaxLen <= 0) {
        return STR_ERR;
    }

    for (i = string->StrLen - 1; i >= 0; i--) {
        if (!isspace(string->Data[i])) {
            if (string->MaxLen > i) {
                string->Data[i + 1] = (unsigned char)'\0';
            }
            string->StrLen = i + 1;
            return STR_OK;
        }
    }

    string->Data[0] = (unsigned char)'\0';
    string->StrLen = 0;
    return STR_OK;
}

/*
 * int Trim(String string)
 *
 * Delete whitespace contiguous from both ends of the string.
 */
int Trim(String string) {
    int i, j;

    if (string == NULL || string->Data == NULL || string->MaxLen < string->StrLen ||
        string->StrLen < 0 || string->MaxLen <= 0) {
        return STR_ERR;
    }

    for (i = string->StrLen - 1; i >= 0; i--) {
        if (!isspace(string->Data[i])) {
            if (string->MaxLen > i) {
                string->Data[i + 1] = (unsigned char)'\0';
            }
            string->StrLen = i + 1;
            for (j = 0; isspace(string->Data[j]); j++) {}
            return Remove(string, 0, j);
        }
    }

    string->Data[0] = (unsigned char)'\0';
    string->StrLen = 0;
    return STR_OK;
}

/*  int EqualsBytes(ConstString string, const void* bytes, int len)
 *
 *  Compare the string with the byte block of length len.  If the
 *  content differs, 0 is returned, if the content is the same, 1 is returned,
 *  if there is an error, -1 is returned.  If the length of the strings are
 *  different, this function is O(1).  '\0' characters are not treated in any
 *  special way.
 */
int EqualsBytes(ConstString string, const void* bytes, int len) {
    if (len < 0 || string == NULL || bytes == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }
    if (string->StrLen != len) {
        return 0;
    }
    if (len == 0 || string->Data == bytes) {
        return 1;
    }
    return !memcmp(string->Data, bytes, len);
}

/*  int Equals(ConstString first, ConstString second)
 *
 *  Compare the strings first and second. If the strings differ, 0 is returned, if
 *  the strings are the same, 1 is returned, if there is an error, -1 is
 *  returned.  If the length of the strings are different, this function is
 *  O(1).  '\0' termination characters are not treated in any special way.
 */
int Equals(ConstString first, ConstString second) {
    if (first == NULL || second == NULL || first->Data == NULL || second->Data == NULL ||
        first->StrLen < 0 || second->StrLen < 0) {
        return STR_ERR;
    }
    if (first->StrLen != second->StrLen) {
        return STR_OK;
    }
    if (first->Data == second->Data || first->StrLen == 0) {
        return 1;
    }
    return !memcmp(first->Data, second->Data, first->StrLen);
}

/*  int StemEqualsBytes(ConstString string, const void* bytes, int len)
 *
 *  Compare beginning of string with a block of memory of length len for
 *  equality.  If the beginning of string differs from the memory block (or if string
 *  is too short), 0 is returned, if the strings are the same, 1 is returned,
 *  if there is an error, -1 is returned.  '\0' characters are not treated in
 *  any special way.
 */
int StemEqualsBytes(ConstString string, const void* bytes, int len) {
    int i;

    if (Data(string) == NULL || string->StrLen < 0 || NULL == bytes || len < 0) {
        return STR_ERR;
    }
    if (string->StrLen < len) {
        return STR_OK;
    }
    if (string->Data == (const unsigned char*)bytes || len == 0) {
        return 1;
    }

    for (i = 0; i < len; i++) {
        if (string->Data[i] != ((const unsigned char*)bytes)[i]) {
            return STR_OK;
        }
    }
    return 1;
}

/*  int EqualsCStr(ConstString string, const char* cStr)
 *
 *  Compare string and char * cStr.  The C string cStr must be '\0'
 *  terminated at exactly the length of the string, and the contents
 *  between the two must be identical with the string with no '\0'
 *  characters for the two contents to be considered equal.  This is
 *  equivalent to the condition that their current contents will be always be
 *  equal when comparing them in the same format after converting one or the
 *  other.  If the strings are equal 1 is returned, if they are unequal 0 is
 *  returned and if there is a detectable error STR_ERR is returned.
 */
int EqualsCStr(ConstString string, const char* cStr) {
    int i;
    if (string == NULL || cStr == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }
    for (i = 0; i < string->StrLen; i++) {
        if (cStr[i] == '\0' || string->Data[i] != (unsigned char)cStr[i]) {
            return STR_OK;
        }
    }
    return cStr[i] == '\0';
}

/*  int EqualsCStrCaseless(ConstString string, const char* cStr)
 *
 *  Compare the string and char * cStr.  The C string cStr must be '\0'
 *  terminated at exactly the length of the string, and the contents
 *  between the two must be identical except for case with the string with
 *  no '\0' characters for the two contents to be considered equal.  This is
 *  equivalent to the condition that their current contents will be always be
 *  equal ignoring case when comparing them in the same format after
 *  converting one or the other.  If the strings are equal, except for case,
 *  1 is returned, if they are unequal regardless of case 0 is returned and
 *  if there is a detectable error STR_ERR is returned.
 */
int EqualsCStrCaseless(ConstString string, const char* cStr) {
    int i;
    if (string == NULL || cStr == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }
    for (i = 0; i < string->StrLen; i++) {
        if (cStr[i] == '\0' || (string->Data[i] != (unsigned char)cStr[i] &&
        tolower(string->Data[i]) != (unsigned char)tolower(cStr[i]))) {
            return STR_OK;
        }
    }
    return cStr[i] == '\0';
}

/*  int StrCompare(ConstString first, ConstString second)
 *
 *  Compare the string first and second.  If there is an error, SHRT_MIN is returned,
 *  otherwise a value less than or greater than zero, indicating that the
 *  string pointed to by first is lexicographically less than or greater than
 *  the string pointed to by second is returned.  If the string lengths are
 *  unequal but the characters up until the length of the shorter are equal
 *  then a value less than, or greater than zero, indicating that the string
 *  pointed to by first is shorter or longer than the string pointed to by second is
 *  returned.  0 is returned if and only if the two strings are the same.  If
 *  the length of the strings are different, this function is O(n).  Like its
 *  standard C library counterpart strcmp, the comparison does not proceed
 *  past any '\0' termination characters encountered.
 */
int StrCompare(ConstString first, ConstString second) {
    int i, v, n;

    if (first == NULL || second == NULL || first->Data == NULL || second->Data == NULL ||
        first->StrLen < 0 || second->StrLen < 0) {
        return SHRT_MIN;
    }
    n = first->StrLen;
    if (n > second->StrLen) {
        n = second->StrLen;
    }
    if (first->StrLen == second->StrLen && (first->Data == second->Data || first->StrLen == 0)) {
        return STR_OK;
    }

    for (i = 0; i < n; i++) {
        v = ((char)first->Data[i]) - ((char)second->Data[i]);
        if (v != 0) {
            return v;
        }
        if (first->Data[i] == (unsigned char)'\0') {
            return STR_OK;
        }
    }

    if (first->StrLen > n) {
        return 1;
    }
    if (second->StrLen > n) {
        return -1;
    }
    return STR_OK;
}

/*  int StrNCompare(ConstString first, ConstString second, int n)
 *
 *  Compare the string first and second for at most n characters.  If there is an
 *  error, SHRT_MIN is returned, otherwise a value is returned as if first and
 *  second were first truncated to at most n characters then StrCompare was called
 *  with these new strings are parameters.  If the length of the strings are
 *  different, this function is O(n).  Like its standard C library counterpart
 *  strcmp, the comparison does not proceed past any '\0' termination
 *  characters encountered.
 */
int StrNCompare(ConstString first, ConstString second, int n) {
    int i, v, m;

    if (first == NULL || second == NULL || first->Data == NULL || second->Data == NULL ||
        first->StrLen < 0 || second->StrLen < 0) {
        return SHRT_MIN;
    }
    m = n;
    if (m > first->StrLen) {
        m = first->StrLen;
    }
    if (m > second->StrLen) {
        m = second->StrLen;
    }

    if (first->Data != second->Data) {
        for (i = 0; i < m; i++) {
            v = ((char)first->Data[i]) - ((char)second->Data[i]);
            if (v != 0) {
                return v;
            }
            if (first->Data[i] == (unsigned char)'\0') {
                return STR_OK;
            }
        }
    }

    if (n == m || first->StrLen == second->StrLen) {
        return STR_OK;
    }

    if (first->StrLen > m) {
        return 1;
    }
    return -1;
}

/*  String Substring(ConstString string, int left, int len)
 *
 *  Create a string which is the substring of string starting from position left
 *  and running for a length len (clamped by the end of the string string.)  If
 *  string is detectably invalid, then NULL is returned.  The section described
 *  by (left, len) is clamped to the boundaries of string.
 */
String Substring(ConstString string, int left, int len) {

    if (string == NULL || string->StrLen < 0 || string->Data == NULL) {
        return NULL;
    }

    if (left < 0) {
        len += left;
        left = 0;
    }

    if (len > string->StrLen - left) {
        len = string->StrLen - left;
    }

    if (len <= 0) {
        return FromCString("");
    }
    return BytesToString(string->Data + left, len);
}

/*  int Remove(String string, int pos, int len)
 *
 *  Removes characters from pos to pos+len-1 inclusive and shifts the tail of
 *  the string starting from pos+len to pos.  len must be positive for this
 *  call to have any effect.  The section of the string described by (pos,
 *  len) is clamped to boundaries of the string.
 */
int Remove(String string, int pos, int len) {
    /* Clamp to left side of string */
    if (pos < 0) {
        len += pos;
        pos = 0;
    }

    if (len < 0 || string == NULL || string->Data == NULL || string->StrLen < 0 ||
        string->MaxLen < string->StrLen || string->MaxLen <= 0) {
        return STR_ERR;
    }
    if (len > 0 && pos < string->StrLen) {
        if (pos + len >= string->StrLen) {
            string->StrLen = pos;
        }
        else {
            if(string->StrLen - (pos + len) > 0) {
                memmove((char*)(string->Data + pos),
                        (char*)(string->Data + pos + len),
                        string->StrLen - (pos + len));
            }
            string->StrLen -= len;
        }
        string->Data[string->StrLen] = (unsigned char)'\0';
    }
    return STR_OK;
}

/*  int Destroy(String string)
 *
 *  Free up the string.  Note that if string is detectably invalid or not writable
 *  then no action is performed and STR_ERR is returned.  Like a freed memory
 *  allocation, dereferences, writes or any other action on string after it has
 *  been destroyed is undefined.
 */
int Destroy(String string) {
    if (string == NULL || string->StrLen < 0 || string->MaxLen <= 0 || string->MaxLen < string->StrLen ||
        string->Data == NULL) {
        return STR_ERR;
    }

    free(string->Data);
    /* In case there is any stale usage, there is one more chance to
       notice this error. */
    string->StrLen = -1;
    string->MaxLen = -1;
    string->Data = NULL;

    free(string);
    return STR_OK;
}

/*  int Contains(ConstString string, int pos, ConstString substring)
 *
 *  Search for the substring in string starting from position pos, and searching
 *  forward.  If it is found then return with the first position where it is
 *  found, otherwise return STR_ERR.  Note that this is just a brute force
 *  string searcher that does not attempt clever things like the Boyer-Moore
 *  search algorithm.  Because of this there are many degenerate cases where
 *  this can take much longer than it needs to.
 */
int Contains(ConstString string, int pos, ConstString substring) {
    int j, ii, ll, lf;
    unsigned char* d0;
    unsigned char c0;
    register unsigned char* d1;
    register unsigned char c1;
    register int i;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 ||
        substring == NULL || substring->Data == NULL || substring->StrLen < 0) {
        return STR_ERR;
    }
    if (string->StrLen == pos) {
        return (substring->StrLen == 0) ? pos : STR_ERR;
    }
    if (string->StrLen < pos || pos < 0) {
        return STR_ERR;
    }
    if (substring->StrLen == 0) {
        return pos;
    }

    /* No space to find such a string? */
    if ((lf = string->StrLen - substring->StrLen + 1) <= pos) {
        return STR_ERR;
    }

    /* An obvious alias case */
    if (string->Data == substring->Data && pos == 0) {
        return 0;
    }

    i = pos;

    d0 = substring->Data;
    d1 = string->Data;
    ll = substring->StrLen;

    /* Peel off the substring->StrLen == 1 case */
    c0 = d0[0];
    if (1 == ll) {
        for (; i < lf; i++) {
            if (c0 == d1[i]) {
                return i;
            }
        }
        return STR_ERR;
    }

    c1 = c0;
    j = 0;
    lf = string->StrLen - 1;

    ii = -1;
    if (i < lf){
        do {
            /* Unrolled current character test */
            if (c1 != d1[i]) {
                if (c1 != d1[1 + i]) {
                    i += 2;
                    continue;
                }
                i++;
            }

            /* Take note if this is the start of a potential match */
            if (0 == j) {
                ii = i;
            }

            /* Shift the test character down by one */
            j++;
            i++;

            /* If this isn't past the last character continue */
            if (j < ll) {
                c1 = d0[j];
                continue;
            }

N0:
            /* If no characters mismatched, then we matched */
            if (i == ii + j) {
                return ii;
            }

            /* Shift back to the beginning */
            i -= j;
            j = 0;
            c1 = c0;
        } while (i < lf);
    }

    /* Deal with last case if unrolling caused a misalignment */
    if (i == lf && ll == j + 1 && c1 == d1[i]) {
        goto N0;
    }

    return STR_ERR;
}

/*  int ContainsReverse(ConstString string, int pos, ConstString substring)
 *
 *  Search for the substring in string starting from position pos, and searching
 *  backward.  If it is found then return with the first position where it is
 *  found, otherwise return STR_ERR.  Note that this is just a brute force
 *  string searcher that does not attempt clever things like the Boyer-Moore
 *  search algorithm.  Because of this there are many degenerate cases where
 *  this can take much longer than it needs to.
 */
int ContainsReverse(ConstString string, int pos, ConstString substring) {
    int j, i, l;
    unsigned char* d0, * d1;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 ||
        substring == NULL || substring->Data == NULL || substring->StrLen < 0) {
        return STR_ERR;
    }
    if (string->StrLen == pos && substring->StrLen == 0) {
        return pos;
    }
    if (string->StrLen < pos || pos < 0) {
        return STR_ERR;
    }
    if (substring->StrLen == 0) {
        return pos;
    }

    /* Obvious alias case */
    if (string->Data == substring->Data && pos == 0 && substring->StrLen <= string->StrLen) {
        return 0;
    }

    i = pos;
    if ((l = string->StrLen - substring->StrLen) < 0) {
        return STR_ERR;
    }

    /* If no space to find such a string then snap back */
    if (l + 1 <= i) {
        i = l;
    }
    j = 0;

    d0 = substring->Data;
    d1 = string->Data;
    l = substring->StrLen;

    for (;;) {
        if (d0[j] == d1[i + j]) {
            j++;
            if (j >= l) {
                return i;
            }
        }
        else {
            i--;
            if (i < 0) {
                break;
            }
            j = 0;
        }
    }

    return STR_ERR;
}

/*  int ContainsCaseless(ConstString string, int pos, ConstString substring)
 *
 *  Search for the substring in string starting from position pos, and searching
 *  forward but without regard to case.  If it is found then return with the
 *  string position where it is found, otherwise return STR_ERR.  Note that
 *  this is just a brute force string searcher that does not attempt clever
 *  things like the Boyer-Moore search algorithm.  Because of this there are
 *  many degenerate cases where this can take much longer than it needs to.
 */
int ContainsCaseless(ConstString string, int pos, ConstString substring) {
    int j, i, l, ll;
    unsigned char* d0, * d1;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 ||
        substring == NULL || substring->Data == NULL || substring->StrLen < 0) {
        return STR_ERR;
    }
    if (string->StrLen == pos) {
        return (substring->StrLen == 0) ? pos : STR_ERR;
    }
    if (string->StrLen < pos || pos < 0) {
        return STR_ERR;
    }
    if (substring->StrLen == 0) {
        return pos;
    }

    l = string->StrLen - substring->StrLen + 1;

    /* No space to find such a string? */
    if (l <= pos) {
        return STR_ERR;
    }

    /* An obvious alias case */
    if (string->Data == substring->Data && pos == 0) {
        return STR_OK;
    }

    i = pos;
    j = 0;

    d0 = substring->Data;
    d1 = string->Data;
    ll = substring->StrLen;

    for (;;) {
        if (d0[j] == d1[i + j] || tolower(d0[j]) == tolower(d1[i + j])) {
            j++;
            if (j >= ll) return i;
        }
        else {
            i++;
            if (i >= l) break;
            j = 0;
        }
    }

    return STR_ERR;
}

/*  int ContainsReverseCaseless(ConstString string, int pos, ConstString substring)
 *
 *  Search for the substring in string starting from position pos, and searching
 *  backward but without regard to case.  If it is found then return with the
 *  first position where it is found, otherwise return STR_ERR.  Note that
 *  this is just a brute force string searcher that does not attempt clever
 *  things like the Boyer-Moore search algorithm.  Because of this there are
 *  many degenerate cases where this can take much longer than it needs to.
 */
int ContainsReverseCaseless(ConstString string, int pos, ConstString substring) {
    int j, i, l;
    unsigned char* d0, * d1;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 ||
        substring == NULL || substring->Data == NULL || substring->StrLen < 0) {
        return STR_ERR;
    }
    if (string->StrLen == pos && substring->StrLen == 0) {
        return pos;
    }
    if (string->StrLen < pos || pos < 0) {
        return STR_ERR;
    }
    if (substring->StrLen == 0) {
        return pos;
    }

    /* Obvious alias case */
    if (string->Data == substring->Data && pos == 0 && substring->StrLen <= string->StrLen) {
        return STR_OK;
    }

    i = pos;
    if ((l = string->StrLen - substring->StrLen) < 0) {
        return STR_ERR;
    }

    /* If no space to find such a string then snap back */
    if (l + 1 <= i) {
        i = l;
    }
    j = 0;

    d0 = substring->Data;
    d1 = string->Data;
    l = substring->StrLen;

    for (;;) {
        if (d0[j] == d1[i + j] || tolower(d0[j]) == tolower(d1[i + j])) {
            j++;
            if (j >= l) {
                return i;
            }
        }
        else {
            i--;
            if (i < 0) {
                break;
            }
            j = 0;
        }
    }

    return STR_ERR;
}


/*  int ContainsCh(ConstString string, int ch, int pos)
 *
 *  Search for the character ch in string forwards from the position pos
 *  (inclusive).
 */
int ContainsCh(ConstString string, int ch, int pos) {
    unsigned char* p;

    if (string == NULL || string->Data == NULL || string->StrLen <= pos || pos < 0) {
        return STR_ERR;
    }
    p = (unsigned char*)memchr((string->Data + pos), (unsigned char)ch, (string->StrLen - pos));
    if (p) {
        return (int)(p - string->Data);
    }
    return STR_ERR;
}

/*  int ContainsChReverse(ConstString string, int ch, int pos)
 *
 *  Search for the character ch in string backwards from the position pos in string
 *  (inclusive).
 */
int ContainsChReverse(ConstString string, int ch, int pos) {
    int i;

    if (string == NULL || string->Data == NULL || string->StrLen <= pos || pos < 0) {
        return STR_ERR;
    }
    for (i = pos; i >= 0; i--) {
        if (string->Data[i] == (unsigned char)ch) return i;
    }
    return STR_ERR;
}

#if !defined (STRLIB_AGGRESSIVE_MEMORY_FOR_SPEED_TRADEOFF)
#define LONG_LOG_BITS_QTY (3)
#define LONG_BITS_QTY (1 << LONG_LOG_BITS_QTY)
#define LONG_TYPE unsigned char

#define CFCLEN ((1 << CHAR_BIT) / LONG_BITS_QTY)
struct charField { LONG_TYPE content[CFCLEN]; };
#define testInCharField(cf,c) ((cf)->content[(c) >> LONG_LOG_BITS_QTY] & \
	                           (((long)1) << ((c) & (LONG_BITS_QTY-1))))
#define setInCharField(cf,idx) { \
	unsigned int c = (unsigned int) (idx); \
	(cf)->content[c >> LONG_LOG_BITS_QTY] |= \
		(LONG_TYPE) (1ul << (c & (LONG_BITS_QTY-1))); \
}
#else
#define CFCLEN (1 << CHAR_BIT)
struct charField { unsigned char content[CFCLEN]; };
#define testInCharField(cf,c) ((cf)->content[(unsigned char) (c)])
#define setInCharField(cf,idx) (cf)->content[(unsigned int) (idx)] = ~0
#endif

/* Convert a string to charField */
static int buildCharField(struct charField* charField, ConstString constString) {
    int i;
    if (constString == NULL || constString->Data == NULL || constString->StrLen <= 0) {
        return STR_ERR;
    }
    memset((void*)charField->content, 0, sizeof(struct charField));
    for (i = 0; i < constString->StrLen; i++) {
        setInCharField(charField, constString->Data[i]);
    }
    return STR_OK;
}

static void invertCharField(struct charField* charField) {
    int i;
    for (i = 0; i < CFCLEN; i++) {
        charField->content[i] = ~charField->content[i];
    }
}

/* Inner engine for byteContainsCharField */
static int byteContainsCharField(const unsigned char* data, int len, int pos,
                                 const struct charField* charField) {
    int i;
    for (i = pos; i < len; i++) {
        unsigned char c = (unsigned char)data[i];
        if (testInCharField(charField, c)) return i;
    }
    return STR_ERR;
}

/*  int Search(ConstString string, int pos, ConstString substring);
 *
 *  Search for the first position in string starting from pos or after, in which
 *  one of the characters in substring is found and return it.  If such a position
 *  does not exist in string, then STR_ERR is returned.
 */
int Search(ConstString string, int pos, ConstString substring) {
    struct charField chrs;
    if (pos < 0 || string == NULL || string->Data == NULL ||
        string->StrLen <= pos) {
        return STR_ERR;
    }
    if (substring->StrLen == 1) {
        return ContainsCh(string, substring->Data[0], pos);
    }
    if (buildCharField(&chrs, substring) < 0) {
        return STR_ERR;
    }
    return byteContainsCharField(string->Data, string->StrLen, pos, &chrs);
}

/* Inner engine for bytesInCharFieldReverse */
static int bytesInCharFieldReverse(const unsigned char* data, int pos,
                                   const struct charField* charField) {
    int i;
    for (i = pos; i >= 0; i--) {
        unsigned int c = (unsigned int)data[i];
        if (testInCharField(charField, c)) {
            return i;
        }
    }
    return STR_ERR;
}

/*  int SearchReverse(ConstString string, int pos, ConstString substring);
 *
 *  Search for the last position in string no greater than pos, in which one of
 *  the characters in substring is found and return it.  If such a position does not
 *  exist in string, then STR_ERR is returned.
 */
int SearchReverse(ConstString string, int pos, ConstString substring) {
    struct charField chrs;
    if (pos < 0 || string == NULL || string->Data == NULL || substring == NULL ||
        string->StrLen < pos) {
        return STR_ERR;
    }
    if (pos == string->StrLen) {
        pos--;
    }
    if (1 == substring->StrLen) {
        return ContainsChReverse(string, substring->Data[0], pos);
    }
    if (0 > buildCharField(&chrs, substring)) {
        return STR_ERR;
    }
    return bytesInCharFieldReverse(string->Data, pos, &chrs);
}

/*  int SearchExclude(ConstString string, int pos, ConstString substring);
 *
 *  Search for the first position in string starting from pos or after, in which
 *  none of the characters in substring is found and return it.  If such a position
 *  does not exist in string, then STR_ERR is returned.
 */
int SearchExclude(ConstString string, int pos, ConstString substring) {
    struct charField chrs;
    if (pos < 0 || string == NULL || string->Data == NULL ||
        string->StrLen <= pos) {
        return STR_ERR;
    }
    if (buildCharField(&chrs, substring) < 0) {
        return STR_ERR;
    }
    invertCharField(&chrs);
    return byteContainsCharField(string->Data, string->StrLen, pos, &chrs);
}

/*  int SearchExcludeReverse(ConstString string, int pos, ConstString substring);
 *
 *  Search for the last position in string no greater than pos, in which none of
 *  the characters in substring is found and return it.  If such a position does not
 *  exist in string, then STR_ERR is returned.
 */
int SearchExcludeReverse(ConstString string, int pos, ConstString substring) {
    struct charField chrs;
    if (pos < 0 || string == NULL || string->Data == NULL ||
        string->StrLen < pos) {
        return STR_ERR;
    }
    if (pos == string->StrLen) {
        pos--;
    }
    if (buildCharField(&chrs, substring) < 0) {
        return STR_ERR;
    }
    invertCharField(&chrs);
    return bytesInCharFieldReverse(string->Data, pos, &chrs);
}

/*  int ReplaceString(String string, int pos, ConstString substring, BOOL fill)
 *
 *  Overwrite the string starting at position pos with the string substring. If
 *  the position pos is past the end of string, then the character "fill" is
 *  appended as necessary to make up the gap between the end of string and pos.
 *  If substring is NULL, it behaves as if it were a 0-length string.
 */
int ReplaceString(String string, int pos, ConstString substring, unsigned char fill) {
    int d, newLen;
    ptrdiff_t pd;
    String aux = (String)substring;

    if (pos < 0 || string == NULL || string->StrLen < 0 || NULL == string->Data ||
        string->MaxLen < string->StrLen || string->MaxLen <= 0) {
        return STR_ERR;
    }
    if (substring != NULL && (substring->StrLen < 0 || substring->Data == NULL)) {
        return STR_ERR;
    }

    d = pos;

    /* Aliasing case */
    if (aux != NULL) {
        if ((pd = (ptrdiff_t)(substring->Data - string->Data)) >= 0 &&
            pd < (ptrdiff_t)string->MaxLen) {
            aux = StrCopy(substring);
            if (aux == NULL) {
                return STR_ERR;
            }
        }
        d += aux->StrLen;
    }

    /* Increase memory size if necessary */
    if (Alloc(string, d + 1) != STR_OK) {
        if (aux != substring) {
            Destroy(aux);
        }
        return STR_ERR;
    }

    newLen = string->StrLen;

    /* Fill in "fill" character as necessary */
    if (pos > newLen) {
        memset(string->Data + string->StrLen,
               (int)fill,
               (size_t)(pos - string->StrLen));
        newLen = pos;
    }

    /* Copy substring to position pos in string. */
    if (aux != NULL) {
        if(aux->StrLen > 0) {
            memmove((char *) (string->Data + pos),
                    (char *) aux->Data,
                    aux->StrLen);
        }
        if (aux != substring) {
            Destroy(aux);
        }
    }

    /* Indicate the potentially increased size of string */
    if (d > newLen) {
        newLen = d;
    }

    string->StrLen = newLen;
    string->Data[newLen] = (unsigned char)'\0';

    return STR_OK;
}

/*  int InsertBytes(String string, int pos, const void* bytes, int len, unsigned char fill)
 *
 *  Inserts the block of characters at bytes with length len into string at position
 *  pos.  If the position pos is past the end of string, then the character "fill"
 *  is appended as necessary to make up the gap between the end of string and pos.
 *  Unlike ReplaceString, Insert does not allow bytes to be NULL.
 */
int InsertBytes(String string, int pos, const void* bytes, int len, unsigned char fill) {
    int d, l;
    unsigned char* aux = (unsigned char*)bytes;

    if (string == NULL || bytes == NULL || pos < 0 || len < 0 || string->StrLen < 0 ||
        string->MaxLen <= 0 || string->MaxLen < string->StrLen) {
        return STR_ERR;
    }

    /* Compute the two possible end pointers */
    d = string->StrLen + len;
    l = pos + len;
    if ((d | l) < 0) {
        return STR_ERR; /* Integer wrap around. */
    }

    /* Aliasing case */
    if (((size_t)((unsigned char*)bytes + len)) >= ((size_t)string->Data) &&
        ((size_t)bytes) < ((size_t)(string->Data + string->MaxLen))) {
        if (NULL == (aux = (unsigned char*)malloc(len))) {
            return STR_ERR;
        }
        memcpy(aux, bytes, len);
    }

    if (l > d) {
        /* Inserting past the end of the string */
        if (Alloc(string, l + 1) != STR_OK) {
            if (aux != (unsigned char*)bytes) {
                free(aux);
            }
            return STR_ERR;
        }
        memset(string->Data + string->StrLen,
               (int)fill,
               (size_t)(pos - string->StrLen));
        string->StrLen = l;
    }
    else {
        /* Inserting in the middle of the string */
        if (Alloc(string, d + 1) != STR_OK) {
            if (aux != (unsigned char*)bytes) {
                free(aux);
            }
            return STR_ERR;
        }
        if(d - l > 0) {
            memmove(string->Data + l,
                    string->Data + pos,
                    d - l);
        }
        string->StrLen = d;
    }
    if(len > 0) {
        memmove(string->Data + pos,
                aux,
                len);
    }
    string->Data[string->StrLen] = (unsigned char)'\0';
    if (aux != (unsigned char*)bytes) {
        free(aux);
    }
    return STR_OK;
}

/*  int Insert(String string, int pos, ConstString substring, unsigned char fill)
 *
 *  Inserts the substring into string at position pos.  If the position pos is
 *  past the end of string, then the character "fill" is appended as necessary to
 *  make up the gap between the end of string and pos.  Unlike ReplaceString, Insert
 *  does not allow substring to be NULL.
 */
int Insert(String string, int pos, ConstString substring, unsigned char fill) {
    if (NULL == substring || (substring->MaxLen > 0 && substring->StrLen > substring->MaxLen)) {
        return STR_ERR;
    }
    return InsertBytes(string, pos, substring->Data, substring->StrLen, fill);
}

/*  int Replace(String string, int pos, int len, ConstString replace, unsigned char fill)
 *
 *  Replace a section of a string from pos for a length len with the string
 *  replace. fill is used is pos > string->StrLen.
 */
int Replace(String string, int pos, int len, ConstString replace, unsigned char fill) {
    int pl, ret;
    ptrdiff_t pd;
    String aux = (String)replace;

    if (pos < 0 || len < 0) {
        return STR_ERR;
    }
    if (pos > INT_MAX - len) {
        return STR_ERR; /* Overflow */
    }
    pl = pos + len;
    if (string == NULL || replace == NULL || string->Data == NULL || replace->Data == NULL ||
        string->StrLen < 0 || replace->StrLen < 0 || string->MaxLen < string->StrLen ||
        string->MaxLen <= 0) {
        return STR_ERR;
    }

    /* Straddles the end? */
    if (pl >= string->StrLen) {
        if ((ret = ReplaceString(string, pos, replace, fill)) < 0) {
            return ret;
        }
        if (pos + replace->StrLen < string->StrLen) {
            string->StrLen = pos + replace->StrLen;
            string->Data[string->StrLen] = (unsigned char)'\0';
        }
        return ret;
    }

    /* Aliasing case */
    if ((pd = (ptrdiff_t)(replace->Data - string->Data)) >= 0 &&
        pd < (ptrdiff_t)string->StrLen) {
        aux = StrCopy(replace);
        if (aux == NULL) {
            return STR_ERR;
        }
    }

    if (aux->StrLen > len) {
        if (Alloc(string, string->StrLen + aux->StrLen - len) != STR_OK) {
            if (aux != replace) {
                Destroy(aux);
            }
            return STR_ERR;
        }
    }

    if (aux->StrLen != len) {
        memmove(string->Data + pos + aux->StrLen,
                string->Data + pos + len,
                string->StrLen - (pos + len));
    }
    memcpy(string->Data + pos,
           aux->Data,
           aux->StrLen);
    string->StrLen += aux->StrLen - len;
    string->Data[string->StrLen] = (unsigned char)'\0';
    if (aux != replace) {
        Destroy(aux);
    }
    return STR_OK;
}

/*
 *  findReplaceEngine is used to implement bfindreplace and
 *  bfindreplacecaseless. It works by breaking the three cases of
 *  expansion, reduction and replacement, and solving each of these
 *  in the most efficient way possible.
 */

typedef int (*findFuncPtr) (ConstString string, int pos, ConstString find);

#define INITIAL_STATIC_FIND_INDEX_COUNT 32

static int findReplaceEngine(String string, ConstString find,
                             ConstString repl, int pos,
                             findFuncPtr pFunction) {
    int i, ret, slen, mlen, delta, acc;
    int* d;
    int static_d[INITIAL_STATIC_FIND_INDEX_COUNT + 1]; /* This +1 is for LINT. */
    ptrdiff_t pd;
    String auxf = (String)find;
    String auxr = (String)repl;

    if (string == NULL || string->Data == NULL || find == NULL ||
        find->Data == NULL || repl == NULL || repl->Data == NULL ||
        pos < 0 || find->StrLen <= 0 || string->MaxLen <= 0 || string->StrLen > string->MaxLen ||
        string->StrLen < 0 || repl->StrLen < 0) {
        return STR_ERR;
    }
    if (pos > string->StrLen - find->StrLen) {
        return STR_OK;
    }

    /* Alias with find string */
    pd = (ptrdiff_t)(find->Data - string->Data);
    if ((ptrdiff_t)(pos - find->StrLen) < pd && pd < (ptrdiff_t)string->StrLen) {
        auxf = StrCopy(find);
        if (auxf == NULL) {
            return STR_ERR;
        }
    }

    /* Alias with repl string */
    pd = (ptrdiff_t)(repl->Data - string->Data);
    if ((ptrdiff_t)(pos - repl->StrLen) < pd && pd < (ptrdiff_t)string->StrLen) {
        auxr = StrCopy(repl);
        if (auxr == NULL) {
            if (auxf != find) {
                Destroy(auxf);
            }
            return STR_ERR;
        }
    }

    delta = auxf->StrLen - auxr->StrLen;

    /* in-place replacement since find and replace strings are of equal
       length */
    if (delta == 0) {
        while ((pos = pFunction(string, pos, auxf)) >= 0) {
            if(auxr->StrLen > 0) {
                memcpy(string->Data + pos, auxr->Data, auxr->StrLen);
            }
            pos += auxf->StrLen;
        }
        if (auxf != find) {
            Destroy(auxf);
        }
        if (auxr != repl) {
            Destroy(auxr);
        }
        return STR_OK;
    }

    /* shrinking replacement since auxf->StrLen > auxr->StrLen */
    if (delta > 0) {
        acc = 0;

        while ((i = pFunction(string, pos, auxf)) >= 0) {
            if (acc && i > pos) {
                memmove (string->Data + pos - acc, string->Data + pos, i - pos);
            }
            if (auxr->StrLen) {
                memcpy(string->Data + i - acc, auxr->Data, auxr->StrLen);
            }
            acc += delta;
            pos = i + auxf->StrLen;
        }

        if (acc) {
            i = string->StrLen;
            if (i > pos) {
                memmove (string->Data + pos - acc, string->Data + pos, i - pos);
            }
            string->StrLen -= acc;
            string->Data[string->StrLen] = (unsigned char)'\0';
        }

        if (auxf != find) {
            Destroy(auxf);
        }
        if (auxr != repl) {
            Destroy(auxr);
        }
        return STR_OK;
    }

    /* expanding replacement since find->StrLen < repl->StrLen.  It's a lot
       more complicated.  This works by first finding all the matches and
       storing them to a growable array, then doing at most one resize of
       the destination string and then performing the direct memory transfers
       of the string segment pieces to form the final result. The growable
       array of matches uses a deferred doubling reallocing strategy.  What
       this means is that it starts as a reasonably fixed sized auto array in
       the hopes that many if not most cases will never need to grow this
       array.  But it switches as soon as the bounds of the array will be
       exceeded.  An extra find result is always appended to this array that
       corresponds to the end of the destination string, so StrLen is checked
       against MaxLen - 1 rather than MaxLen before resizing.
    */

    mlen = INITIAL_STATIC_FIND_INDEX_COUNT;
    d = (int*)static_d; /* Avoid malloc for trivial/initial cases */
    acc = slen = 0;

    while ((pos = pFunction(string, pos, auxf)) >= 0) {
        if (slen >= mlen - 1) {
            int* t;
            int sl;
            /* Overflow */
            if (mlen > (INT_MAX / sizeof(int*)) / 2) {
                ret = STR_ERR;
                goto done;
            }
            mlen += mlen;
            sl = sizeof(int*) * mlen;
            if (static_d == d) {
                d = NULL; /* static_d cannot be realloced */
            }
            if (NULL == (t = (int*)realloc(d, sl))) {
                ret = STR_ERR;
                goto done;
            }
            if (NULL == d) {
                memcpy(t, static_d, sizeof(static_d));
            }
            d = t;
        }
        d[slen] = pos;
        slen++;
        acc -= delta;
        pos += auxf->StrLen;
        if (pos < 0 || acc < 0) {
            ret = STR_ERR;
            goto done;
        }
    }

    /* StrLen <= INITIAL_STATIC_INDEX_COUNT-1 or MaxLen-1 here. */
    d[slen] = string->StrLen;

    if (STR_OK == (ret = Alloc(string, string->StrLen + acc + 1))) {
        string->StrLen += acc;
        for (i = slen - 1; i >= 0; i--) {
            int s, l;
            s = d[i] + auxf->StrLen;
            l = d[i + 1] - s; /* d[StrLen] may be accessed here. */
            if (l) {
                memmove (string->Data + s + acc, string->Data + s, l);
            }
            if (auxr->StrLen) {
                memmove (string->Data + s + acc - auxr->StrLen,
                         auxr->Data, auxr->StrLen);
            }
            acc += delta;
        }
        string->Data[string->StrLen] = (unsigned char)'\0';
    }

    done:;
    if (static_d != d) {
        free(d);
    }
    if (auxf != find) {
        Destroy(auxf);
    }
    if (auxr != repl) {
        Destroy(auxr);
    }
    return ret;
}

/*  int FindReplace(String string, ConstString find, ConstString repl, int pos)
 *
 *  Replace all occurrences of a find string with a replace string after a
 *  given point in a string.
 */
int FindReplace(String string, ConstString find, ConstString repl, int pos) {
    return findReplaceEngine(string, find, repl, pos, Contains);
}

/*  int FindReplaceCaseless(String string, ConstString find, ConstString repl, int pos)
 *
 *  Replace all occurrences of a find string, ignoring case, with a replace
 *  string after a given point in a bstring.
 */
int FindReplaceCaseless(String string, ConstString find, ConstString repl, int pos) {
    return findReplaceEngine(string, find, repl, pos, ContainsCaseless);
}

/*  int InsertCh(String string, int pos, int len, unsigned char fill)
 *
 *  Inserts the character fill repeatedly into string at position pos for a
 *  length len.  If the position pos is past the end of string, then the
 *  character "fill" is appended as necessary to make up the gap between the
 *  end of string and the position pos + len.
 */
int InsertCh(String string, int pos, int len, unsigned char fill) {
    int d, l, i;

    if (pos < 0 || string == NULL || string->StrLen < 0 || string->MaxLen < string->StrLen ||
        string->MaxLen <= 0 || len < 0) {
        return STR_ERR;
    }

    /* Compute the two possible end pointers */
    d = string->StrLen + len;
    l = pos + len;
    if ((d | l) < 0) {
        return STR_ERR;
    }

    if (l > d) {
        /* Inserting past the end of the string */
        if (Alloc(string, l + 1) != STR_OK) {
            return STR_ERR;
        }
        pos = string->StrLen;
        string->StrLen = l;
    }
    else {
        /* Inserting in the middle of the string */
        if (Alloc(string, d + 1) != STR_OK) {
            return STR_ERR;
        }
        for (i = d - 1; i >= l; i--) {
            string->Data[i] = string->Data[i - len];
        }
        string->StrLen = d;
    }

    for (i = pos; i < l; i++) {
        string->Data[i] = fill;
    }
    string->Data[string->StrLen] = (unsigned char)'\0';
    return STR_OK;
}

/*  int Pattern(String string, int len)
 *
 *  Replicate a string, string in place, end to end repeatedly until it
 *  surpasses len characters, then chop the result to exactly len characters.
 *  This function operates in-place.  The function will return with STR_ERR
 *  if string is NULL or of length 0, otherwise STR_OK is returned.
 */
int Pattern(String string, int len) {
    int i, d;

    d = Length(string);
    if (d <= 0 || len < 0 || Alloc(string, len + 1) != STR_OK) {
        return STR_ERR;
    }
    if (len > 0) {
        if (d == 1) {
            return ReplaceString(string, len, NULL, string->Data[0]);
        }
        for (i = d; i < len; i++) {
            string->Data[i] = string->Data[i - d];
        }
    }
    string->Data[len] = (unsigned char)'\0';
    string->StrLen = len;
    return STR_OK;
}

#ifndef START_VSNBUFF
#define START_VSNBUFF (16)
#endif

#if defined (__TURBOC__) && !defined (__BORLANDC__)
# ifndef STRLIB_NOVSNP
#  define STRLIB_NOVSNP
# endif
#endif
/* Give WATCOM C/C++, MSVC some latitude for their non-support of vsnprintf */
#if defined(__WATCOMC__) || defined(_MSC_VER)
#define exvsnprintf(r,b,n,f,a) {r = _vsnprintf (b,n,f,a);}
#else
#ifdef STRLIB_NOVSNP
/* This is just a hack.  If you are using a system without a vsnprintf, it is
   not recommended that FormatString be used at all. */
#define exvsnprintf(r,b,n,f,a) {vsprintf (b,f,a); r = -1;}
#define START_VSNBUFF (256)
#else
#if defined(__GNUC__) && !defined(__APPLE__)
/* Something is making gcc complain about this prototype not being here, so
   I've just gone ahead and put it in. */
extern int vsnprintf (char *buf, size_t count, const char *format, va_list arg);
#endif
#define exvsnprintf(r,b,n,f,a) {r = vsnprintf (b,n,f,a);}
#endif
#endif

/*  int FormatString(String string, const char* format, ...)
 *
 *  After the first parameter, it takes the same parameters as printf (), but
 *  rather than outputting results to stdio, it appends the results to
 *  a string which contains what would have been output. Note that if there
 *  is an early generation of a '\0' character, the string will be truncated
 *  to this end point.
 */
int FormatString(String string, const char* format, ...) {
    va_list arglist;
    String buff;
    int n, r;

    if (string == NULL || format == NULL || string->Data == NULL || string->MaxLen <= 0
        || string->StrLen < 0 || string->StrLen > string->MaxLen) {
        return STR_ERR;
    }

    /* Since the length is not determinable beforehand, a search is
       performed using the truncating "vsnprintf" call (to avoid buffer
       overflows) on increasing potential sizes for the output result. */
    if ((n = (int)(2 * strlen(format))) < START_VSNBUFF) {
        n = START_VSNBUFF;
    }
    if (NULL == (buff = FromCStrAlloc(n + 2, ""))) {
        n = 1;
        if (NULL == (buff = FromCStrAlloc(n + 2, ""))) {
            return STR_ERR;
        }
    }

    for (;;) {
                va_start(arglist, format);
        exvsnprintf(r, (char*)buff->Data, n + 1, format, arglist);
                va_end(arglist);

        buff->Data[n] = (unsigned char)'\0';
        buff->StrLen = (int)(strlen)((char*)buff->Data);

        if (buff->StrLen < n) break;

        if (r > n) n = r; else n += n;

        if (STR_OK != Alloc(buff, n + 2)) {
            Destroy(buff);
            return STR_ERR;
        }
    }

    r = Concat(string, buff);
    Destroy(buff);
    return r;
}

/*  int AssignFormat(String string, const char* fmt, ...)
 *
 *  After the first parameter, it takes the same parameters as printf (), but
 *  rather than outputting results to stdio, it outputs the results to
 *  the string parameter. Note that if there is an early generation of a
 *  '\0' character, the string will be truncated to this end point.
 */
int AssignFormat(String string, const char* fmt, ...) {
    va_list arglist;
    String buff;
    int n, r;

    if (string == NULL || fmt == NULL || string->Data == NULL || string->MaxLen <= 0
        || string->StrLen < 0 || string->StrLen > string->MaxLen) {
        return STR_ERR;
    }

    /* Since the length is not determinable beforehand, a search is
       performed using the truncating "vsnprintf" call (to avoid buffer
       overflows) on increasing potential sizes for the output result. */

    if ((n = (int)(2 * strlen(fmt))) < START_VSNBUFF) {
        n = START_VSNBUFF;
    }
    if (NULL == (buff = FromCStrAlloc(n + 2, ""))) {
        n = 1;
        if (NULL == (buff = FromCStrAlloc(n + 2, ""))) {
            return STR_ERR;
        }
    }

    for (;;) {
                va_start(arglist, fmt);
        exvsnprintf(r, (char*)buff->Data, n + 1, fmt, arglist);
                va_end(arglist);

        buff->Data[n] = (unsigned char)'\0';
        buff->StrLen = (int)(strlen)((char*)buff->Data);

        if (buff->StrLen < n) {
            break;
        }

        if (r > n) {
            n = r;
        } else {
            n += n;
        }

        if (STR_OK != Alloc(buff, n + 2)) {
            Destroy(buff);
            return STR_ERR;
        }
    }

    r = Assign(string, buff);
    Destroy(buff);
    return r;
}

/*  String Format(const char* fmt, ...)
 *
 *  Takes the same parameters as printf (), but rather than outputting results
 *  to stdio, it forms a string which contains what would have been output.
 *  Note that if there is an early generation of a '\0' character, the
 *  string will be truncated to this end point.
 */
String Format(const char* fmt, ...) {
    va_list arglist;
    String buff;
    int n, r;

    if (fmt == NULL) {
        return NULL;
    }

    /* Since the length is not determinable beforehand, a search is
       performed using the truncating "vsnprintf" call (to avoid buffer
       overflows) on increasing potential sizes for the output result. */

    if ((n = (int)(2 * strlen(fmt))) < START_VSNBUFF) {
        n = START_VSNBUFF;
    }
    if (NULL == (buff = FromCStrAlloc(n + 2, ""))) {
        n = 1;
        if (NULL == (buff = FromCStrAlloc(n + 2, ""))) {
            return NULL;
        }
    }

    for (;;) {
                va_start(arglist, fmt);
        exvsnprintf(r, (char*)buff->Data, n + 1, fmt, arglist);
                va_end(arglist);

        buff->Data[n] = (unsigned char)'\0';
        buff->StrLen = (int)(strlen)((char*)buff->Data);

        if (buff->StrLen < n) {
            break;
        }

        if (r > n) {
            n = r;
        } else {
            n += n;
        }

        if (STR_OK != Alloc(buff, n + 2)) {
            Destroy(buff);
            return NULL;
        }
    }

    return buff;
}

/*  int VcFormat(String string, int count, const char* fmt, va_list arg)
 *
 *  The VcFormat function formats Data under control of the format control
 *  string fmt and attempts to append the result to string.  The fmt parameter is
 *  the same as that of the printf function.  The variable argument list is
 *  replaced with arglist, which has been initialized by the va_start macro.
 *  The size of the output is upper bounded by count.  If the required output
 *  exceeds count, the string is not augmented with any contents and a value
 *  below STR_ERR is returned.  If a value below -count is returned then it
 *  is recommended that the negative of this value be used as an update to the
 *  count in a subsequent pass.  On other errors, such as running out of
 *  memory, parameter errors or numeric wrap around STR_ERR is returned.
 *  STR_OK is returned when the output is successfully generated and
 *  appended to string.
 *
 *  Note: There is no sanity checking of arglist, and this function is
 *  destructive of the contents of string from the string->StrLen point onward.  If there
 *  is an early generation of a '\0' character, the string will be truncated
 *  to this end point.
 */
int VcFormat(String string, int count, const char* fmt, va_list arg) {
    int n, r, l;

    if (string == NULL || fmt == NULL || count <= 0 || string->Data == NULL
        || string->MaxLen <= 0 || string->StrLen < 0 || string->StrLen > string->MaxLen) {
        return STR_ERR;
    }

    if (count > (n = string->StrLen + count) + 2) {
        return STR_ERR;
    }
    if (STR_OK != Alloc(string, n + 2)) {
        return STR_ERR;
    }

    exvsnprintf(r, (char*)string->Data + string->StrLen, count + 2, fmt, arg);
    string->Data[string->StrLen + count + 2] = '\0';

    /* Did the operation complete successfully within bounds? */

    if (n >= (l = string->StrLen + (int)(strlen)((char*)string->Data + string->StrLen))) {
        string->StrLen = l;
        return STR_OK;
    }

    /* Abort, since the buffer was not large enough.  The return value
       tries to help set what the retry length should be. */

    string->Data[string->StrLen] = '\0';
    if (r > count + 1) {
        l = r;
    } else {
        if (count > INT_MAX / 2) {
            l = INT_MAX;
        }
        else {
            l = count + count;
        }
    }
    n = -l;
    if (n > STR_ERR - 1) {
        n = STR_ERR - 1;
    }
    return n;
}