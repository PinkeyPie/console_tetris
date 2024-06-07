#pragma once

extern char* (gets)(char* buf);
extern char* (strncpy)(char* dst, const char* src, size_t n);
extern char* (strncat)(char* dst, const char* src, size_t n);
extern char* (strtok)(char* s1, const char* s2);
extern char* (strdup)(const char* s);
#undef strcpy
#undef strcat
#define strcpy(a,b) bsafe_strcpy(a,b)
#define strcat(a,b) bsafe_strcat(a,b) 