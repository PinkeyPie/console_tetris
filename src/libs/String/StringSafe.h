#pragma once
#include "StringGlobal.h"

STRING_EXPORT char* (gets)(char* buf);
STRING_EXPORT char* (strncpy)(char* dst, const char* src, size_t n);
STRING_EXPORT char* (strncat)(char* dst, const char* src, size_t n);
STRING_EXPORT char* (strtok)(char* s1, const char* s2);
STRING_EXPORT char* (strdup)(const char* s);
#undef strcpy
#undef strcat
#define strcpy(a,b) bsafe_strcpy(a,b)
#define strcat(a,b) bsafe_strcat(a,b) 