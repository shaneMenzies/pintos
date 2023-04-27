/**
 * @file process_def.cpp
 * @author Shane Menzies
 * @brief
 * @date 2/8/23
 *
 *
 */

#include "process_def.h"

#include "memory/p_memory.h"
#include "threading.h"

namespace threading {
process::process(unsigned int target_priority, unsigned int rounds,
                 std_k::callable<void>* target, process* parent)
    : priority(target_priority)
    , rounds(rounds)
    , main(target)
    , out_stream(active_terminal)
    , parent_task(parent) {

    // Set appropriate address space for this task
    if (parent_task == nullptr) {
        task_space = new paging::address_space();
    } else {
        // Link to parent task's address space
        task_space = new paging::address_space(parent_task->task_space);

        // Set this into the parent task's child tasks
        parent_task = common_region::current_process;
        parent_task->children.push_back(this);
    }

    // Need to add new table to address space
    lvl4_table = task_space->primary_table;

    // Create and set this task's own stack
    // Stack will need space for the red zone below
    // Stack needs to by 16-byte aligned for extended floating point operations
    user_stack   = aligned_alloc(DEFAULT_STACK_SIZE, 16);
    kernel_stack = aligned_alloc(DEFAULT_STACK_SIZE, 16);
    saved_state.rsp
        = (uint64_t)((uintptr_t)user_stack + DEFAULT_STACK_SIZE + 128);

    // Init wrapper assumes itself to be a function call,
    // so it expects the stack to be offset by 8 bytes, for the return address
    saved_state.push(0);

    // Map this process info into the process' address space
    task_space->map_region_to((uintptr_t)this,
                              (uintptr_t)common_region::current_process,
                              sizeof(process));

    // Get id from process list
    pid = process_list.add_process(this);

    // Set initial target
    prepare_wrapper();
}

process::~process() {

    if (parent_task != nullptr) {
        // Remove from parent task
        for (size_t index = 0; index < parent_task->children.size(); index++) {
            if (parent_task->children[index] == this) {
                parent_task->children.erase(index);
                break;
            }
        }
    }

    // Remove from process list
    process_list.remove_process(pid);

    // Free stack space
    free(user_stack);
}

void process::init_wrapper(process* target) {
    while (target->rounds != 0) {
        // Call actual target
        target->main->call();

        if (target->rounds > 0) target->rounds--;
    }

    // Delete task by yielding remaining time
    asm volatile("int $0xa1");
}
} // namespace threading
