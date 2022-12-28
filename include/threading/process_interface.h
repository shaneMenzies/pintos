#ifndef PROCESS_INTERFACE_H
#define PROCESS_INTERFACE_H

#include "memory/common_region.h"
#include "threading/threading.h"

namespace this_process {
// This interface operates on the process mapped into the common region,
// which should be mapped to whatever process is active

timer<>* get_timer() {
    return common_region::current_process->scheduler->local_timer;
}

void yield();

// Sleep will always use lazy timing method,
// will only continue on this process' time share
void sleep(double seconds);

} // namespace this_process

#endif // PROCESS_INTERFACE_H
