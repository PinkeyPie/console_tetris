#include <stdio.h>
#include <stdlib.h>
#include "StringSafe.h"

static int bsafeShouldExit = 1;

char* strcpy(char* dst, const char* src);
char* strcat(char* dst, const char* src);

char* strcpy(char* dst, const char* src) {
    (void)dst;
    (void)src;
    fprintf(stderr, "bsafe error: strcpy() is not safe, use bstrcpy instead.\n");
    if (bsafeShouldExit) exit(-1);
    return NULL;
}

char* strcat(char* dst, const char* src) {
    (void)dst;
    (void)src;
    fprintf(stderr, "bsafe error: strcat() is not safe, use bstrcat instead.\n");
    if (bsafeShouldExit) exit(-1);
    return NULL;
}

char* (strncpy)(char* dst, const char* src, size_t n) {
    (void)dst;
    (void)src;
    (void)n;
    fprintf(stderr, "bsafe error: strncpy() is not safe, use bmidstr instead.\n");
    if (bsafeShouldExit) exit(-1);
    return NULL;
}

char* (strncat)(char* dst, const char* src, size_t n) {
    (void)dst;
    (void)src;
    (void)n;
    fprintf(stderr, "bsafe error: strncat() is not safe, use bstrcat then btrunc\n\tor cstr2tbstr, btrunc then bstrcat instead.\n");
    if (bsafeShouldExit) exit(-1);
    return NULL;
}

char* (strtok)(char* s1, const char* s2) {
    (void)s1;
    (void)s2;
    fprintf(stderr, "bsafe error: strtok() is not safe, use bsplit or bsplits instead.\n");
    if (bsafeShouldExit) exit(-1);
    return NULL;
}

char* (strdup)(const char* s) {
    (void)s;
    fprintf(stderr, "bsafe error: strdup() is not safe, use bstrcpy.\n");
    if (bsafeShouldExit) exit(-1);
    return NULL;
}