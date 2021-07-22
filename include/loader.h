#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include <stddef.h>

#include "multiboot.h"
#include "pintos_std.h"
#include "elf_64.h"

#define FB_DEFAULT_WIDTH 480
#define FB_DEFAULT_HEIGHT 360
#define FB_DEFAULT_DEPTH 32

const char kernel_mod_identifier[] = "pintos_kernel_64";

#endif