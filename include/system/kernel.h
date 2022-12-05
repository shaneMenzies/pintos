#ifndef KERNEL_H
#define KERNEL_H

#include "memory/common_region.h"
#include "time/timer.h"

#include <stdint.h>

struct terminal;

extern terminal* log_terminal;

extern timer<>* sys_int_timer;

#endif
