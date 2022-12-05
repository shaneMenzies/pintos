#ifndef PINTOS_STDLIB_H
#define PINTOS_STDLIB_H

#include "string.h"

#include <stdint.h>

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void  free(void* target);

int atexit(void* func);

int atoi(const char* str);

__attribute__((noreturn)) void abort();
char*                          getenv(const char* env_var);

int abs(int n);

#endif // PINTOS_STDLIB_H
