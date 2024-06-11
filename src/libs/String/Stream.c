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
#include "Stream.h"


#define BS_BUFF_SZ (1024)

/*  int ReadAppend(String string, ReadFuncPtr readFunc, void* parm)
 *
 *  Use a finite buffer fread-like function readFunc to concatenate to the
 *  string the entire contents of file-like source Data in a roughly
 *  efficient way.
 */
int ReadAppend(String string, ReadFuncPtr readFunc, void* parm) {
    int i, l, n;

    if (string == NULL || string->MaxLen <= 0 || string->StrLen < 0 || string->MaxLen < string->StrLen ||
        readFunc == NULL) {
        return STR_ERR;
    }

    i = string->StrLen;
    for (n = i + 16; ; n += ((n < BS_BUFF_SZ) ? n : BS_BUFF_SZ)) {
        if (STR_OK != Alloc(string, n + 1)) {
            return STR_ERR;
        }
        l = (int)readFunc((void*)(string->Data + i), 1, n - i, parm);
        i += l;
        string->StrLen = i;
        if (i < n) {
            break;
        }
    }

    string->Data[i] = (unsigned char)'\0';
    return STR_OK;
}

/*  String Read(ReadFuncPtr readPtr, void* parm)
 *
 *  Use a finite buffer fread-like function readPtr to create a String
 *  filled with the entire contents of file-like source Data in a roughly
 *  efficient way.
 */
String Read(ReadFuncPtr readPtr, void* parm) {
    String buff;

    if (0 > ReadAppend(buff = FromCString(""), readPtr, parm)) {
        Destroy(buff);
        return NULL;
    }
    return buff;
}

/*  int AssignGetStr(String string, GetCharFuncPtr getCharFunc, void* parm, char terminator)
 *
 *  Use an fgetc-like single character stream reading function (getCharFunc) to
 *  obtain a sequence of characters which are concatenated to the end of the
 *  string.  The stream read is terminated by the passed in terminator
 *  parameter.
 *
 *  If getCharFunc returns with a negative number, or the terminator character
 *  (which is appended) is read, then the stream reading is halted and the
 *  function returns with a partial result in string.  If there is an empty partial
 *  result, 1 is returned.  If no characters are read, or there is some other
 *  detectable error, STR_ERR is returned.
 */
int AssignGetStr(String string, GetCharFuncPtr getCharFunc, void* parm, char terminator) {
    int c, d, e;

    if (string == NULL || string->MaxLen <= 0 || string->StrLen < 0 || string->MaxLen < string->StrLen ||
        getCharFunc == NULL) {
        return STR_ERR;
    }
    d = 0;
    e = string->MaxLen - 2;

    while ((c = getCharFunc(parm)) >= 0) {
        if (d > e) {
            string->StrLen = d;
            if (Alloc(string, d + 2) != STR_OK) {
                return STR_ERR;
            }
            e = string->MaxLen - 2;
        }
        string->Data[d] = (unsigned char)c;
        d++;
        if (c == terminator) {
            break;
        }
    }

    string->Data[d] = (unsigned char)'\0';
    string->StrLen = d;

    return d == 0 && c < 0;
}

/*  int bgetsa (bstring string, GetCharFuncPtr getCharFuncPtr, void * parm, char terminator)
 *
 *  Use an fgetc-like single character stream reading function (getCharFuncPtr) to
 *  obtain a sequence of characters which are concatenated to the end of the
 *  bstring string.  The stream read is terminated by the passed in terminator
 *  parameter.
 *
 *  If getCharFuncPtr returns with a negative number, or the terminator character
 *  (which is appended) is read, then the stream reading is halted and the
 *  function returns with a partial result concatenated to string.  If there is
 *  an empty partial result, 1 is returned.  If no characters are read, or
 *  there is some other detectable error, STR_ERR is returned.
 */
int GetCharAppend(String string, GetCharFuncPtr getCharFuncPtr, void* parm, char terminator) {
    int c, d, e;

    if (string == NULL || string->MaxLen <= 0 || string->StrLen < 0 || string->MaxLen < string->StrLen ||
        getCharFuncPtr == NULL) {
        return STR_ERR;
    }
    d = string->StrLen;
    e = string->MaxLen - 2;

    while ((c = getCharFuncPtr(parm)) >= 0) {
        if (d > e) {
            string->StrLen = d;
            if (Alloc(string, d + 2) != STR_OK) {
                return STR_ERR;
            }
            e = string->MaxLen - 2;
        }
        string->Data[d] = (unsigned char)c;
        d++;
        if (c == terminator) {
            break;
        }
    }

    string->Data[d] = (unsigned char)'\0';
    string->StrLen = d;

    return d == 0 && c < 0;
}

/*  String GetChar(GetCharFuncPtr getCharFunc, void* parm, char terminator)
 *
 *  Use an fgetc-like single character stream reading function (getCharFunc) to
 *  obtain a sequence of characters which are concatenated into a string.
 *  The stream read is terminated by the passed in terminator function.
 *
 *  If getCharFunc returns with a negative number, or the terminator character
 *  (which is appended) is read, then the stream reading is halted and the
 *  result obtained thus far is returned.  If no characters are read, or
 *  there is some other detectable error, NULL is returned.
 */
String GetChar(GetCharFuncPtr getCharFunc, void* parm, char terminator) {
    String buff;

    if (0 > GetCharAppend(buff = FromCString(""), getCharFunc, parm, terminator) ||
        0 >= buff->StrLen) {
        Destroy(buff);
        buff = NULL;
    }
    return buff;
}

/*  struct Stream* StreamOpen(ReadFuncPtr readPtr, void* parm)
 *
 *  Wrap a given open stream (described by a fread compatible function
 *  pointer and stream handle) into an open bStream suitable for the string
 *  library streaming functions.
 */
Stream StreamOpen(ReadFuncPtr readPtr, void* parm) {
    Stream stream;

    if (readPtr == NULL) {
        return NULL;
    }
    stream = (Stream)malloc(sizeof(struct _Stream));
    if (stream == NULL) {
        return NULL;
    }
    stream->parm = parm;
    stream->buff = FromCString("");
    stream->readFnPtr = readPtr;
    stream->maxBuffSz = BS_BUFF_SZ;
    stream->isEOF = 0;
    return stream;
}

/*  int StreamBuffLength(struct Stream* stream, int size)
 *
 *  SetAt the length of the buffer used by the bStream.  If size is zero, the
 *  length is not set.  This function returns with the previous length.
 */
int StreamBuffLength(Stream stream, int size) {
    int oldSz;
    if (stream == NULL || size < 0) {
        return STR_ERR;
    }
    oldSz = stream->maxBuffSz;
    if (size > 0) {
        stream->maxBuffSz = size;
    }
    return oldSz;
}

int StreamEOF(ConstStream stream) {
    if (stream == NULL || stream->readFnPtr == NULL) {
        return STR_ERR;
    }
    return stream->isEOF && (stream->buff->StrLen == 0);
}

/*  void * bsclose (struct bStream * stream)
 *
 *  Close the bStream, and return the handle to the stream that was originally
 *  used to open the given stream.
 */
void* StreamClose(Stream stream) {
    void* parm;
    if (stream == NULL) {
        return NULL;
    }
    stream->readFnPtr = NULL;
    if (stream->buff) {
        Destroy(stream->buff);
    }
    stream->buff = NULL;
    parm = stream->parm;
    stream->parm = NULL;
    stream->isEOF = 1;
    free(stream);
    return parm;
}

/*  int StreamReadLineAppend(String reader, struct Stream* stream, char terminator)
 *
 *  Read a string terminated by the terminator character or the end of the
 *  stream from the bStream (stream) and return it into the parameter reader.  This
 *  function may read additional characters from the core stream that are not
 *  returned, but will be retained for subsequent read operations.
 */
int StreamReadLineAppend(String reader, Stream stream, char terminator) {
    int i, l, ret, rlo;
    char* bytes;
    struct _TagString x;

    if (stream == NULL || stream->buff == NULL || reader == NULL || reader->MaxLen <= 0 ||
        reader->StrLen < 0 || reader->MaxLen < reader->StrLen) {
        return STR_ERR;
    }
    l = stream->buff->StrLen;
    if (STR_OK != Alloc(stream->buff, stream->maxBuffSz + 1)) {
        return STR_ERR;
    }
    bytes = (char*)stream->buff->Data;
    x.Data = (unsigned char*)bytes;

    /* First check if the current buffer holds the terminator */
    bytes[l] = terminator; /* SetAt sentinel */
    for (i = 0; bytes[i] != terminator; i++);
    if (i < l) {
        x.StrLen = i + 1;
        ret = Concat(reader, &x);
        stream->buff->StrLen = l;
        if (STR_OK == ret) {
            Remove(stream->buff, 0, i + 1);
        }
        return STR_OK;
    }

    rlo = reader->StrLen;

    /* If not then just concatenate the entire buffer to the output */
    x.StrLen = l;
    if (STR_OK != Concat(reader, &x)) {
        return STR_ERR;
    }

    /* Perform direct in-place reads into the destination to allow for
       the minimum of Data-copies */
    for (;;) {
        if (STR_OK != Alloc(reader, reader->StrLen + stream->maxBuffSz + 1)) {
            return STR_ERR;
        }
        bytes = (char*)(reader->Data + reader->StrLen);
        l = (int)stream->readFnPtr(bytes, 1, stream->maxBuffSz, stream->parm);
        if (l <= 0) {
            reader->Data[reader->StrLen] = (unsigned char)'\0';
            stream->buff->StrLen = 0;
            stream->isEOF = 1;
            /* If nothing was read return with an error message */
            return STR_ERR & -(reader->StrLen == rlo);
        }
        bytes[l] = terminator; /* SetAt sentinel */
        for (i = 0; bytes[i] != terminator; i++);
        if (i < l) {
            break;
        }
        reader->StrLen += l;
    }

    /* Terminator found, push over-read back to buffer */
    i++;
    reader->StrLen += i;
    stream->buff->StrLen = l - i;
    memcpy(stream->buff->Data, bytes + i, l - i);
    reader->Data[reader->StrLen] = (unsigned char)'\0';
    return STR_OK;
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

/*  int StreamReadLinesAppend(String reader, struct Stream* stream, ConstString term)
 *
 *  Read a string terminated by any character in the term string or the end
 *  of the stream from the bStream (stream) and return it into the parameter reader.
 *  This function may read additional characters from the core stream that
 *  are not returned, but will be retained for subsequent read operations.
 */
int StreamReadLinesAppend(String reader, Stream stream, ConstString term) {
    int i, l, ret, rlo;
    unsigned char* b;
    struct _TagString x;
    struct charField cf;

    if (stream == NULL || stream->buff == NULL || reader == NULL || term == NULL ||
        term->Data == NULL || reader->MaxLen <= 0 || reader->StrLen < 0 ||
        reader->MaxLen < reader->StrLen) {
        return STR_ERR;
    }
    if (term->StrLen == 1) {
        return StreamReadLineAppend(reader, stream, term->Data[0]);
    }
    if (term->StrLen < 1 || buildCharField(&cf, term)) {
        return STR_ERR;
    }

    l = stream->buff->StrLen;
    if (STR_OK != Alloc(stream->buff, stream->maxBuffSz + 1)) {
        return STR_ERR;
    }
    b = (unsigned char*)stream->buff->Data;
    x.Data = b;

    /* First check if the current buffer holds the terminator */
    b[l] = term->Data[0]; /* SetAt sentinel */
    for (i = 0; !testInCharField(&cf, b[i]); i++);
    if (i < l) {
        x.StrLen = i + 1;
        ret = Concat(reader, &x);
        stream->buff->StrLen = l;
        if (STR_OK == ret) {
            Remove(stream->buff, 0, i + 1);
        }
        return STR_OK;
    }

    rlo = reader->StrLen;

    /* If not then just concatenate the entire buffer to the output */
    x.StrLen = l;
    if (STR_OK != Concat(reader, &x)) {
        return STR_ERR;
    }

    /* Perform direct in-place reads into the destination to allow for
       the minimum of Data-copies */
    for (;;) {
        if (STR_OK != Alloc(reader, reader->StrLen + stream->maxBuffSz + 1)) {
            return STR_ERR;
        }
        b = (unsigned char*)(reader->Data + reader->StrLen);
        l = (int)stream->readFnPtr(b, 1, stream->maxBuffSz, stream->parm);
        if (l <= 0) {
            reader->Data[reader->StrLen] = (unsigned char)'\0';
            stream->buff->StrLen = 0;
            stream->isEOF = 1;
            /* If nothing was read return with an error message */
            return STR_ERR & -(reader->StrLen == rlo);
        }

        b[l] = term->Data[0]; /* SetAt sentinel */
        for (i = 0; !testInCharField(&cf, b[i]); i++);
        if (i < l) {
            break;
        }
        reader->StrLen += l;
    }

    /* Terminator found, push over-read back to buffer */
    i++;
    reader->StrLen += i;
    stream->buff->StrLen = l - i;
    memcpy(stream->buff->Data, b + i, l - i);
    reader->Data[reader->StrLen] = (unsigned char)'\0';
    return STR_OK;
}

/*  int StreamReadAppend(String string, struct Stream* stream, int n)
 *
 *  Read a string of length n (or, if it is fewer, as many bytes as is
 *  remaining) from the bStream.  This function may read additional
 *  characters from the core stream that are not returned, but will be
 *  retained for subsequent read operations.  This function will not read
 *  additional characters from the core stream beyond virtual stream pointer.
 */
int StreamReadAppend(String string, Stream stream, int n) {
    int l, ret, orslen;
    char* bytes;
    struct _TagString x;

    if (stream == NULL || stream->buff == NULL || string == NULL || string->MaxLen <= 0
        || string->StrLen < 0 || string->MaxLen < string->StrLen || n <= 0) {
        return STR_ERR;
    }

    if (n > INT_MAX - string->StrLen) {
        return STR_ERR;
    }
    n += string->StrLen;

    l = stream->buff->StrLen;

    orslen = string->StrLen;

    if (0 == l) {
        if (stream->isEOF) {
            return STR_ERR;
        }
        if (string->MaxLen > n) {
            l = (int)stream->readFnPtr(string->Data + string->StrLen, 1, n - string->StrLen,
                                       stream->parm);
            if (0 >= l || l > n - string->StrLen) {
                stream->isEOF = 1;
                return STR_ERR;
            }
            string->StrLen += l;
            string->Data[string->StrLen] = (unsigned char)'\0';
            return 0;
        }
    }

    if (STR_OK != Alloc(stream->buff, stream->maxBuffSz + 1)) {
        return STR_ERR;
    }
    string = (char*)stream->buff->Data;
    x.Data = (unsigned char*)string;

    do {
        if (l + string->StrLen >= n) {
            x.StrLen = n - string->StrLen;
            ret = Concat(string, &x);
            stream->buff->StrLen = l;
            if (STR_OK == ret) {
                Remove(stream->buff, 0, x.StrLen);
            }
            return STR_ERR & -(string->StrLen == orslen);
        }

        x.StrLen = l;
        if (STR_OK != Concat(string, &x)) {
            break;
        }

        l = n - string->StrLen;
        if (l > stream->maxBuffSz) {
            l = stream->maxBuffSz;
        }

        l = (int)stream->readFnPtr(string, 1, l, stream->parm);

    } while (l > 0);
    if (l < 0) {
        l = 0;
    }
    if (l == 0) {
        stream->isEOF = 1;
    }
    stream->buff->StrLen = l;
    return STR_ERR & -(string->StrLen == orslen);
}

/*  int StreamReadLine(String string, struct Stream* stream, char terminator)
 *
 *  Read a string terminated by the terminator character or the end of the
 *  stream from the bStream (stream) and return it into the parameter string.  This
 *  function may read additional characters from the core stream that are not
 *  returned, but will be retained for subsequent read operations.
 */
int StreamReadLine(String string, Stream stream, char terminator) {
    if (stream == NULL || stream->buff == NULL || string == NULL || string->MaxLen <= 0) {
        return STR_ERR;
    }
    if (STR_OK != Alloc(stream->buff, stream->maxBuffSz + 1)) {
        return STR_ERR;
    }
    string->StrLen = 0;
    return StreamReadLineAppend(string, stream, terminator);
}

/*  int StreamReadLines(String reader, struct Stream* stream, ConstString term)
 *
 *  Read a string terminated by any character in the term string or the end
 *  of the stream from the bStream (stream) and return it into the parameter reader.
 *  This function may read additional characters from the core stream that
 *  are not returned, but will be retained for subsequent read operations.
 */
int StreamReadLines(String reader, Stream stream, ConstString term) {
    if (stream == NULL || stream->buff == NULL || reader == NULL || term == NULL
        || term->Data == NULL || reader->MaxLen <= 0) {
        return STR_ERR;
    }
    if (term->StrLen == 1) {
        return StreamReadLine(reader, stream, term->Data[0]);
    }
    if (term->StrLen < 1) {
        return STR_ERR;
    }
    if (STR_OK != Alloc(stream->buff, stream->maxBuffSz + 1)) {
        return STR_ERR;
    }
    reader->StrLen = 0;
    return StreamReadLinesAppend(reader, stream, term);
}

/*  int StreamRead(String reader, struct Stream* stream, int n)
 *
 *  Read a string of length n (or, if it is fewer, as many bytes as is
 *  remaining) from the bStream.  This function may read additional
 *  characters from the core stream that are not returned, but will be
 *  retained for subsequent read operations.  This function will not read
 *  additional characters from the core stream beyond virtual stream pointer.
 */
int StreamRead(String reader, Stream stream, int n) {
    if (stream == NULL || stream->buff == NULL || reader == NULL || reader->MaxLen <= 0 || n <= 0) {
        return STR_ERR;
    }
    if (STR_OK != Alloc(stream->buff, stream->maxBuffSz + 1)) {
        return STR_ERR;
    }
    reader->StrLen = 0;
    return StreamReadAppend(reader, stream, n);
}

/*  int StreamUnread(struct Stream* stream, ConstString terminator)
 *
 *  Insert a string into the Stream at the current position.  These
 *  characters will be read prior to those that actually come from the core
 *  stream.
 */
int StreamUnread(Stream stream, ConstString terminator) {
    if (stream == NULL || stream->buff == NULL) {
        return STR_ERR;
    }
    return Insert(stream->buff, 0, terminator, (unsigned char)'?');
}

/*  int StreamPeek(String reader, const struct Stream* stream)
 *
 *  Return the currently buffered characters from the bStream that will be
 *  read prior to reads from the core stream.
 */
int StreamPeek(String reader, ConstStream stream) {
    if (stream == NULL || stream->buff == NULL) {
        return STR_ERR;
    }
    return Assign(reader, stream->buff);
}

#define BSSSC_BUFF_LEN (256)

/*  int bssplitscb (struct bStream * stream, const_bstring splitStr,
 *                  int (* cb) (void * parm, int ofs, const_bstring entry),
 *                  void * parm)
 *
 *  Iterate the set of disjoint sequential substrings read from a stream
 *  divided by any of the characters in splitStr.  An empty splitStr causes
 *  the whole stream to be iterated once.
 *
 *  Note: At the point of calling the cb function, the bStream pointer is
 *  pointed exactly at the position right after having read the split
 *  character.  The cb function can act on the stream by causing the bStream
 *  pointer to move, and bssplitscb will continue by starting the next split
 *  at the position of the pointer after the return from cb.
 *
 *  However, if the cb causes the bStream stream to be destroyed then the cb must
 *  return with a negative value, otherwise bssplitscb will continue in an
 *  undefined manner.
 */
int StreamSplitsCb(Stream stream, ConstString splitStr,
                   int (*cb) (void* parm, int ofs, ConstString entry), void* parm) {
    struct charField chrs;
    String buff;
    int i, p, ret;

    if (cb == NULL || stream == NULL || stream->readFnPtr == NULL ||
        splitStr == NULL || splitStr->StrLen < 0) return STR_ERR;

    if (NULL == (buff = FromCString(""))) return STR_ERR;

    if (splitStr->StrLen == 0) {
        while (StreamReadAppend(buff, stream, BSSSC_BUFF_LEN) >= 0);
        if ((ret = cb(parm, 0, buff)) > 0)
            ret = 0;
    }
    else {
        buildCharField(&chrs, splitStr);
        ret = p = i = 0;
        for (;;) {
            if (i >= buff->StrLen) {
                StreamReadAppend(buff, stream, BSSSC_BUFF_LEN);
                if (i >= buff->StrLen) {
                    if (0 < (ret = cb(parm, p, buff))) ret = 0;
                    break;
                }
            }
            if (testInCharField(&chrs, buff->Data[i])) {
                struct _TagString t;
                unsigned char c;

                StringFromBytes(t, buff->Data + i + 1, buff->StrLen - (i + 1));
                if ((ret = StreamUnread(stream, &t)) < 0) break;
                buff->StrLen = i;
                c = buff->Data[i];
                buff->Data[i] = (unsigned char)'\0';
                if ((ret = cb(parm, p, buff)) < 0) break;
                buff->Data[i] = c;
                buff->StrLen = 0;
                p += i + 1;
                i = -1;
            }
            i++;
        }
    }

    Destroy(buff);
    return ret;
}

/*  int bssplitstrcb (struct bStream * s, const_bstring splitStr,
 *                    int (* cb) (void * parm, int ofs, const_bstring entry),
 *                    void * parm)
 *
 *  Iterate the set of disjoint sequential substrings read from a stream
 *  divided by the entire substring splitStr.  An empty splitStr causes
 *  each character of the stream to be iterated.
 *
 *  Note: At the point of calling the cb function, the bStream pointer is
 *  pointed exactly at the position right after having read the split
 *  character.  The cb function can act on the stream by causing the bStream
 *  pointer to move, and bssplitscb will continue by starting the next split
 *  at the position of the pointer after the return from cb.
 *
 *  However, if the cb causes the bStream s to be destroyed then the cb must
 *  return with a negative value, otherwise bssplitscb will continue in an
 *  undefined manner.
 */
int StreamSplitsStringCb(Stream s, ConstString splitStr, int (*cb) (void* parm, int ofs, ConstString entry), void* parm) {
    String buff;
    int i, p, ret;

    if (cb == NULL || s == NULL || s->readFnPtr == NULL
        || splitStr == NULL || splitStr->StrLen < 0) return STR_ERR;

    if (splitStr->StrLen == 1) return StreamSplitsCb(s, splitStr, cb, parm);

    if (NULL == (buff = FromCString(""))) return STR_ERR;

    if (splitStr->StrLen == 0) {
        for (i = 0; StreamReadAppend(buff, s, BSSSC_BUFF_LEN) >= 0; i++) {
            if ((ret = cb(parm, 0, buff)) < 0) {
                Destroy(buff);
                return ret;
            }
            buff->StrLen = 0;
        }
        return STR_OK;
    }
    else {
        ret = p = i = 0;
        for (i = p = 0;;) {
            if ((ret = Contains(buff, 0, splitStr)) >= 0) {
                struct _TagString t;
                StringFromBytes(t, buff->Data, ret);
                i = ret + splitStr->StrLen;
                if ((ret = cb(parm, p, &t)) < 0) break;
                p += i;
                Remove(buff, 0, i);
            }
            else {
                StreamReadAppend(buff, s, BSSSC_BUFF_LEN);
                if (StreamEOF(s)) {
                    if ((ret = cb(parm, p, buff)) > 0) ret = 0;
                    break;
                }
            }
        }
    }

    Destroy(buff);
    return ret;
}
