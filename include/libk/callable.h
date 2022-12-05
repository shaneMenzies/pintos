#ifndef PINT_CALLABLE_H
#define PINT_CALLABLE_H

#include "common.h"
#include "libk/asm.h"

namespace std_k {

template<typename Return> struct callable {
  public:
    virtual Return operator()() const = 0;
    virtual Return call() const       = 0;
};

} // namespace std_k

#endif
