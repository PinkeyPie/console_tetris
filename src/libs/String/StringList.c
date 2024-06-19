#include "StringList.h"

HANDLE CreateStringList(void) {
    return NULL;
}

int StringListDestroy(HANDLE stringList) {
    return 0;
}

/* String split and join functions */
HANDLE Split(ConstString str, unsigned char splitChar) {
    return NULL;
}

HANDLE SplitByString(ConstString str, ConstString splitStr) {
    return NULL;
}

String Join(HANDLE stringList, ConstString sep) {
    return NULL;
}

String JoinBytes(HANDLE stringList, const void* bytes, int len) {
    return NULL;
}