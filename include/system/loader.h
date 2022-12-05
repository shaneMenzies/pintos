#ifndef LOADER_H
#define LOADER_H

#include <stddef.h>
#include <stdint.h>

#define FB_DEFAULT_WIDTH  480
#define FB_DEFAULT_HEIGHT 360
#define FB_DEFAULT_DEPTH  32

constexpr char kernel_mod_identifier[] = "pintos_kernel_64";

extern "C" {
extern void* thread_startup_target_code;
extern void* next_thread_stack_top;
extern void  thread_startup();
extern char  thread_startup_end;
}

#endif