#ifndef PINTOS_TIMABLE_DEVICE_H
#define PINTOS_TIMABLE_DEVICE_H

#include "device/device.h"
#include "libk/common.h"
#include "time/timer.h"

template<bool oneshot = false, bool periodic = false>
struct timable_device
    : public virtual device
    , public virtual timer<> {
    using task                                          = timer<>::task;
    using timestamp                                     = timer<>::timestamp;
    virtual timestamp now() const                       = 0;
    virtual timestamp time_to_next() const              = 0;
    virtual timestamp convert_sec(double seconds) const = 0;
    virtual timestamp convert_rate(uint64_t rate) const = 0;

    virtual void set_interrupt_relative(timestamp offset)   = 0;
    virtual void set_interrupt_absolute(timestamp absolute) = 0;

    virtual void set_interrupt_periodic(timestamp interval) = 0;

    timable_device() {}
};

#endif // PINTOS_TIMABLE_DEVICE_H
