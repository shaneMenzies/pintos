/**
 * @file random.cpp
 * @author Shane Menzies
 * @brief
 * @date 11/13/22
 *
 *
 */

#include "random.h"

#include "asm.h"
#include "misc.h"

#include <cpuid.h>

namespace std_k {

bool rdseed_capable() {
    uint32_t eax = 7;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;

    __get_cpuid(7, &eax, &ebx, &ecx, &edx);

    return (ebx & (1 << 18));
}

bool rdrand_capable() {
    uint32_t eax = 1;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;

    __get_cpuid(1, &eax, &ebx, &ecx, &edx);

    return (ecx & (1 << 30));
}

uint64_t shift_register;

void seed_rand() {
    if (rdseed_capable()) {
        shift_register = rd_seed();
    } else if (rdrand_capable()) {
        shift_register = rd_rand();
    } else {
        shift_register = shift_register ^ *((uint64_t*)1) ^ 0xCBA2D3C1DE;
    }
}

uint64_t get_rand() {
    bool input = (shift_register & (1ULL << 63))
        ^ (shift_register & (1ULL << 62))
        ^ (shift_register & (1ULL << 60))
        ^ (shift_register & (1ULL << 59));

    shift_register <<= 1;
    shift_register |= (input ? 1 : 0);

    return shift_register;
}

} // namespace std_k
