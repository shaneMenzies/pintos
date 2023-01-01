#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "process_def.h"
namespace threading {
struct process;
}

namespace common_region {

void* const common_region_start = (void*)0xfffffeffc0000000;

/**
 * Common objects or identifiers made available to all processes through
 *  the kernel address space.
 */

// Current process, should default to fake kernel process
threading::process* const current_process
    = (threading::process*)(common_region_start);

} // namespace common_region
#endif // MEMORY_MAP_H
