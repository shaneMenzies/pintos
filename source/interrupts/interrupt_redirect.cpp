//
// Created by srmenzies on 8/8/22.
//

#include "interrupt_redirect.h"

namespace interrupts {

std_k::callable<void>* irq_redirect_target[256];

}
