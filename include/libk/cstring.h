#ifndef PINT_CSTRING_H
#define PINT_CSTRING_H

#include "common.h"

namespace std_k {
char*  strcpy(char* __restrict__ dest, const char* __restrict__ src);
size_t strlen(const char* str);
int    strcmp(const char* lhs, const char* rhs);
char*  strrev(char* dest, const char* src);

char* strncpy(char* __restrict__ dest, const char* __restrict__ src, size_t n);
int   strncmp(const char* lhs, const char* rhs, size_t n);
char* strnrev(char* dest, const char* src, size_t n);

void*                   memset(void* dest, int ch, size_t count);
template<class T> void* memset_t(void* dest, T value, size_t count);
void* memcpy(void* __restrict__ dest, const void* __restrict__ src,
             size_t count);
void* memmove(void* dest, const void* src, size_t count);
int   memcmp(const void* lhs, const void* rhs, size_t count);

} // namespace std_k

#endif
