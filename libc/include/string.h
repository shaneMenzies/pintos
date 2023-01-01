#ifndef PINTOS_STRING_H
#define PINTOS_STRING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef uint64_t size_t;

void*  memcpy(void*, const void*, size_t);
void*  memset(void*, int, size_t);
char*  strcpy(char*, const char*);
size_t strlen(const char*);
#ifdef __cplusplus
}
#endif

#endif // PINTOS_STRING_H
