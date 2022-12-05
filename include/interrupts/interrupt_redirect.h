//
// Created by srmenzies on 8/8/22.
//

#ifndef PINTOS_INTERRUPT_REDIRECT_H
#define PINTOS_INTERRUPT_REDIRECT_H

#include "libk/callable.h"

namespace interrupts {

extern std_k::callable<void>* irq_redirect_target[256];
inline void set_interrupt(uint8_t irq, std_k::callable<void>* target) {
    irq_redirect_target[irq] = target;
}
} // namespace interrupts

#endif // PINTOS_INTERRUPT_REDIRECT_H
