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

// Increase the size of the memory backing the string to at least length.
int Alloc(String string, int length) {
    if (string == NULL || string->Data == NULL || string->StrLen < 0 || length <= 0) {
        return STR_ERR;
    }
    if(length > string->StrLen) {
        unsigned char *tempMem = (unsigned char *) realloc(string->Data, length + 1);
        if(tempMem == NULL) {
            return STR_ERR;
        }
        string->Data = tempMem;
        string->StrLen = length;
        string->Data[string->StrLen] = '\0';
    } else { // Let's not charge the fortune, by realloc to smaller size
        unsigned char* tempMem = (unsigned char*) malloc((length + 1) * sizeof(unsigned char));
        if(tempMem == NULL) {
            return STR_ERR;
        }
        memcpy(tempMem, string->Data, length);
        tempMem[length] = '\0';
        free(string->Data);
        string->Data = tempMem;
        string->StrLen = length;
    }

    return STR_OK;
}

/*  String FromCString(const char* str)
 *
 *  Create a string which contains the contents of the '\0' terminated char *
 *  buffer str.
 */
String FromCString(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t length = strlen(str);

    String string = (String)malloc(sizeof(struct _TagString));
    if (string == NULL) {
        return NULL;
    }
    string->StrLen = (int)length;
    string->Data = (unsigned char*)malloc((string->StrLen + 1) * sizeof(unsigned char));
    if (string->Data == NULL) {
        free(string);
        return NULL;
    }

    memcpy(string->Data, str, length + 1);
    return string;
}

/*  String BytesToString(const void* bytes, int len)
 *
 *  Create a string which contains the content of the block bytes of length
 *  len.
 */
String BytesToString(const void* bytes, int len) {
    if (bytes == NULL || len < 0) {
        return NULL;
    }
    String string = (String)malloc(sizeof(struct _TagString));
    if (string == NULL) {
        return NULL;
    }
    string->StrLen = len;
    string->Data = (unsigned char*)malloc(((size_t)string->StrLen + 1) * sizeof(unsigned char));
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

/*  char* StringToCString(ConstString string)
 *
 *  Create a '\0' terminated char * buffer which is equal to the contents of
 *  the string s. This returned value should be freed with a
 *  free() call, by the calling application.
 */
char* StringToCString(ConstString string) {
    if (string == NULL || string->StrLen < 0 || string->Data == NULL) {
        return NULL;
    }
    int length = string->StrLen;
    char* result = (char*)malloc((size_t)(length + 1) * sizeof(char));
    if (result == NULL) {
        return result;
    }
    memcpy(result, string->Data, length);
    result[length] = (unsigned char)'\0';

    return result;
}

/*  int Concat(String first, ConstString second)
 *
 *  Concatenate the String second to the String first.
 */
int Concat(String first, ConstString second) {
    String aux = (String)second;
    if (first == NULL || second == NULL || first->Data == NULL || second->Data == NULL) {
        return STR_ERR;
    }

    int dstLen = first->StrLen;
    int srcLen = second->StrLen;
    if(srcLen > 0) {
        Alloc(first, dstLen + srcLen + 1);
        memmove(&(first->Data[dstLen]), &(aux->Data[0]), (size_t)srcLen);
        first->Data[dstLen + srcLen] = (unsigned char)'\0';
        first->StrLen = dstLen + srcLen;
    }

    return STR_OK;
}

/*  int ConcatCh(String string, char ch)
 *
 *  Concatenate the single character ch to the string.
 */
int ConcatCh(String string, char ch) {
    if (string == NULL) {
        return STR_ERR;
    }
    int dstLen = string->StrLen;
    Alloc(string, dstLen + 1);
    string->Data[dstLen] = (unsigned char)ch;
    string->Data[dstLen + 1] = (unsigned char)'\0';
    string->StrLen++;
    return STR_OK;
}

/*  int ConcatCString(String string, const char* str)
 *
 *  Concatenate a char * string to a String.
 */
int ConcatCString(String string, const char* str) {
    char* dest;
    int i, len;

    if (string == NULL || string->Data == NULL || string->StrLen < 0 || str == NULL) {
        return STR_ERR;
    }

    size_t dstLen = (size_t)string->StrLen;
    size_t srcLen = strlen(str);
    Alloc(string, dstLen + srcLen + 1);
    memcpy(&(string->Data[dstLen]), str, srcLen);
    dest = (char*)&string->Data[string->StrLen];
    string->StrLen = dstLen + srcLen;

    return STR_OK;
}

/*  int ConcatBytes(String string, const void* bytes, int len)
 *
 *  Concatenate a fixed length buffer to a string.
 */
int ConcatBytes(String string, const void* bytes, int len) {
    if (string == NULL || string->Data == NULL || string->StrLen < 0 || bytes == NULL || len < 0) {
        return STR_ERR;
    }

    int newLen = string->StrLen + len;
    if (newLen < 0) {
        return STR_ERR;
    }
    Alloc(string, newLen);
    memmove(&string->Data[string->StrLen], bytes, (size_t) len);
    string->StrLen = newLen;
    string->Data[newLen] = (unsigned char)'\0';
    return STR_OK;
}

/*  String StrCopy(ConstString string)
 *
 *  Create a copy of the String string.
 */
String StrCopy(ConstString string) {
    /* Attempted to copy an invalid string? */
    if (string == NULL || string->StrLen < 0 || string->Data == NULL) {
        return NULL;
    }
    String copyStr = (String)malloc(sizeof(struct _TagString));
    if (copyStr == NULL) {
        /* Unable to allocate memory for string header */
        return NULL;
    }
    int length = string->StrLen;
    copyStr->Data = (unsigned char*)malloc(length + 1);
    if (copyStr->Data == NULL) {
        /* Unable to allocate memory for string Data */
        free(copyStr);
        return NULL;
    }
    copyStr->StrLen = length;
    if (length) {
        memcpy((char*)copyStr->Data, (char*)string->Data, length);
    }
    copyStr->Data[copyStr->StrLen] = (unsigned char)'\0';

    return copyStr;
}

/*  int Truncate(String string, int n)
 *
 *  Truncate the string to at most n characters.
 */
int Truncate(String string, int n) {
    if (n < 0 || string == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }
    if (string->StrLen > n) {
        Alloc(string, n);
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
    if (string == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }
    for (int i = 0, len = string->StrLen; i < len; i++) {
        string->Data[i] = (unsigned char)toupper(string->Data[i]);
    }
    return STR_OK;
}

/*  int ToLower(String string)
 *
 *  Convert contents of string to lower case.
 */
int ToLower(String string) {
    if (string == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }
    for (int i = 0, len = string->StrLen; i < len; i++) {
        string->Data[i] = (unsigned char)tolower(string->Data[i]);
    }
    return STR_OK;
}

/*  int StrCompareCaseless(ConstString first, ConstString second)
 *
 *  Compare two strings without differentiating between case.  The return
 *  value is the difference of the values of the characters where the two
 *  strings first differ after lower case transformation, otherwise 0 is
 *  returned indicating that the strings are equal.  If the lengths are
 *  different, then a difference from 0 is given, but if the first extra
 *  character is '\0', then it is taken to be the value UCHAR_MAX+1.
 */
int StrCompareCaseless(ConstString first, ConstString second) {
    int result, n;

    if (Data(first) == NULL || first->StrLen < 0 || Data(second) == NULL || second->StrLen < 0) {
        return SHRT_MIN;
    }
    if ((n = first->StrLen) > second->StrLen) {
        n = second->StrLen;
    } else if (first->StrLen == second->StrLen && first->Data == second->Data) {
        return STR_OK;
    }

    for (int i = 0; i < n; i++) {
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
BOOL EqualsCaselessBytes(ConstString string, const void* bytes, int len) {
    int i;

    if (Data(string) == NULL || string->StrLen < 0 || bytes == NULL || len < 0) {
        return STR_ERR;
    }
    if (string->StrLen != len) {
        return FALSE;
    }
    if (len == 0 || string->Data == bytes) {
        return TRUE;
    }
    for (i = 0; i < len; i++) {
        if (string->Data[i] != ((unsigned char*)bytes)[i]) {
            unsigned char c = (unsigned char)tolower(string->Data[i]);
            if (c != (unsigned char)tolower(((unsigned char*)bytes)[i])) {
                return FALSE;
            }
        }
    }
    return TRUE;
}


/*  int EqualsCaseless(ConstString first, ConstString second)
 *
 *  Compare two strings for equality without differentiating between case.
 *  If the strings differ other than in case, 0 is returned, if the strings
 *  are the same, 1 is returned, if there is an error, -1 is returned.  If
 *  the length of the strings are different, this function is O(1).  '\0'
 *  termination characters are not treated in any special way.
 */
BOOL EqualsCaseless(ConstString first, ConstString second) {
    if (second == NULL) {
        return FALSE;
    }
    return EqualsCaselessBytes(first, second->Data, second->StrLen);
}

/*  int EqualsNCaselessBytes(ConstString string, const void* bytes, int len)
 *
 *  Compare beginning of string with a block of memory of length len
 *  without differentiating between case for equality.  If the beginning of string
 *  differs from the memory block other than in case (or if string is too short),
 *  0 is returned, if the strings are the same, 1 is returned, if there is an
 *  error, -1 is returned.  '\0' characters are not treated in any special
 *  way.
 */
BOOL EqualsNCaselessBytes(ConstString string, const void* bytes, int len) {
    int i;

    if (Data(string) == NULL || string->StrLen < 0 || NULL == bytes || len < 0) {
        return FALSE;
    }
    if (string->StrLen < len) {
        return FALSE;
    }
    if (string->Data == (const unsigned char*)bytes || len == 0) {
        return TRUE;
    }

    for (i = 0; i < len; i++) {
        if (string->Data[i] != ((const unsigned char*)bytes)[i]) {
            if (tolower(string->Data[i]) != tolower(((const unsigned char*)bytes)[i])) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*
 * int LTrim(String string)
 *
 * Delete whitespace contiguous from the left end of the string.
 */
int LTrim(String string) {
    if (string == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }

    for (int len = string->StrLen, i = 0; i < len; i++) {
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
    if (string == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }

    for (int i = string->StrLen - 1; i >= 0; i--) {
        if (!isspace(string->Data[i])) {
            if (string->StrLen > i) {
                string->Data[i + 1] = (unsigned char)'\0';
            }
            Alloc(string, i + 1);
            string->StrLen = i + 1;
            return STR_OK;
        }
    }

    Alloc(string, 1);
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

    if (string == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }

    for (i = string->StrLen - 1; i >= 0; i--) {
        if (!isspace(string->Data[i])) {
            if (string->StrLen > i) {
                string->Data[i + 1] = (unsigned char)'\0';
            }
            string->StrLen = i + 1;
            Alloc(string, string->StrLen);
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
BOOL EqualsBytes(ConstString string, const void* bytes, int len) {
    if (len < 0 || string == NULL || bytes == NULL || string->Data == NULL || string->StrLen < 0) {
        return FALSE;
    }
    if (string->StrLen != len) {
        return FALSE;
    }
    if (len == 0 || string->Data == bytes) {
        return TRUE;
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
BOOL Equals(ConstString first, ConstString second) {
    if (first == NULL || second == NULL || first->Data == NULL || second->Data == NULL ||
        first->StrLen < 0 || second->StrLen < 0) {
        return FALSE;
    }
    if (first->StrLen != second->StrLen) {
        return FALSE;
    }
    if (first->Data == second->Data || first->StrLen == 0) {
        return TRUE;
    }
    return !memcmp(first->Data, second->Data, first->StrLen);
}

/*  int EqualsNBytes(ConstString string, const void* bytes, int len)
 *
 *  Compare beginning of string with a block of memory of length len for
 *  equality.  If the beginning of string differs from the memory block (or if string
 *  is too short), 0 is returned, if the strings are the same, 1 is returned,
 *  if there is an error, -1 is returned.  '\0' characters are not treated in
 *  any special way.
 */
BOOL EqualsNBytes(ConstString string, const void* bytes, int len) {
    if (Data(string) == NULL || string->StrLen < 0 || NULL == bytes || len < 0) {
        return FALSE;
    }
    if (string->StrLen < len) {
        return FALSE;
    }
    if (string->Data == (const unsigned char*)bytes || len == 0) {
        return TRUE;
    }

    for (int i = 0; i < len; i++) {
        if (string->Data[i] != ((const unsigned char*)bytes)[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/*  int EqualsCString(ConstString string, const char* cStr)
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
BOOL EqualsCString(ConstString string, const char* cStr) {
    int i;
    if (string == NULL || cStr == NULL || string->Data == NULL || string->StrLen < 0) {
        return FALSE;
    }
    for (i = 0; i < string->StrLen; i++) {
        if (cStr[i] == '\0' || string->Data[i] != (unsigned char)cStr[i]) {
            return FALSE;
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
    int v, n;

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

    for (int i = 0; i < n; i++) {
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
    int v, m;

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
        for (int i = 0; i < m; i++) {
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

    if (len < 0 || string == NULL || string->Data == NULL || string->StrLen < 0) {
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
        Alloc(string, string->StrLen);
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
    if (string == NULL || string->StrLen < 0 || string->Data == NULL) {
        return STR_ERR;
    }

    free(string->Data);
    /* In case there is any stale usage, there is one more chance to
       notice this error. */
    string->StrLen = -1;
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
    unsigned char* d1;
    unsigned char c1;
    int i;

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

/*  int Search(ConstString string, int pos, ConstString substring);
 *
 *  Search for the first position in string starting from pos or after, in which
 *  one of the characters in substring is found and return it.  If such a position
 *  does not exist in string, then STR_ERR is returned.
 */
int Search(ConstString string, int pos, ConstString substring) {
    // Made it stupid
    return 0;
}

/*  int SearchExclude(ConstString string, int pos, ConstString substring);
 *
 *  Search for the first position in string starting from pos or after, in which
 *  none of the characters in substring is found and return it.  If such a position
 *  does not exist in string, then STR_ERR is returned.
 */
int SearchExclude(ConstString string, int pos, ConstString substring) {
    // Just make it
    return 0;
}

/*  int ReplaceString(String string, int pos, ConstString substring, BOOL fill)
 *
 *  Overwrite the string starting at position pos with the string substring. If
 *  the position pos is past the end of string, then the character "fill" is
 *  appended as necessary to make up the gap between the end of string and pos.
 *  If substring is NULL, it behaves as if it were a 0-length string.
 */
int ReplaceString(String string, int pos, ConstString substring) {
    int newLen;
    if(string == NULL) {
        return STR_ERR;
    }
    if (pos < 0 || pos > string->StrLen || string->StrLen < 0 || NULL == string->Data) {
        return STR_ERR;
    }
    if (substring == NULL) {
        return STR_ERR;
    }
    if(substring->StrLen < 0 || substring->Data == NULL) {
        return STR_ERR;
    }

    int dest = pos + substring->StrLen;
    /* Increase memory size if necessary */
    if(string->StrLen < dest) {
        if (Alloc(string, dest) != STR_OK) {
            return STR_ERR;
        }
    }
    newLen = string->StrLen;

    /* Copy substring to position pos in string. */
    if(substring->StrLen > 0) {
        memmove((char *) (string->Data + pos),
                (char *) substring->Data,
                substring->StrLen);
    }

    /* Indicate the potentially increased size of string */
    if (dest > newLen) {
        newLen = dest;
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

    if (string == NULL || bytes == NULL || pos < 0 || len < 0 || string->StrLen < 0) {
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
        ((size_t)bytes) < ((size_t)(string->Data + string->StrLen))) {
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
    if (NULL == substring) {
        return STR_ERR;
    }
    return InsertBytes(string, pos, substring->Data, substring->StrLen, fill);
}

/*  int Replace(String string, int pos, int len, ConstString replace, unsigned char fill)
 *
 *  Replace a section of a string from pos for a length len with the string
 *  replace. fill is used is pos > string->StrLen.
 */
int Replace(String string, int pos, int len, ConstString replace) {
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
        string->StrLen < 0 || replace->StrLen < 0) {
        return STR_ERR;
    }

    /* Straddles the end? */
    if (pl >= string->StrLen) {
        if ((ret = ReplaceString(string, pos, replace)) < 0) {
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

/*  int FindReplace(String string, ConstString find, ConstString repl, int pos)
 *
 *  Replace all occurrences of a find string with a replace string after a
 *  given point in a string.
 */
int FindReplace(String string, ConstString find, ConstString repl, int pos) {
    // Make it
    return 0;
}

/*  int FindReplaceCaseless(String string, ConstString find, ConstString repl, int pos)
 *
 *  Replace all occurrences of a find string, ignoring case, with a replace
 *  string after a given point in a bstring.
 */
int FindReplaceCaseless(String string, ConstString find, ConstString repl, int pos) {
    // Make it
    return 0;
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

    if (pos < 0 || string == NULL || string->StrLen < 0 || len < 0) {
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

    if (string == NULL || format == NULL || string->Data == NULL || string->StrLen < 0) {
        return STR_ERR;
    }

    /* Since the length is not determinable beforehand, a search is
       performed using the truncating "vsnprintf" call (to avoid buffer
       overflows) on increasing potential sizes for the output result. */
    if ((n = (int)(2 * strlen(format))) < START_VSNBUFF) {
        n = START_VSNBUFF;
    }
    Alloc(buff, n + 2);
    if (NULL == buff) {
        n = 1;
        Alloc(buff, n + 2);
        if (NULL == buff) {
            return STR_ERR;
        }
    }
    buff->Data[0] = '\0';

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
    Alloc(buff, n + 2);
    if (NULL == buff) {
        n = 1;
        Alloc(buff, n + 2);
        if (NULL == buff) {
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