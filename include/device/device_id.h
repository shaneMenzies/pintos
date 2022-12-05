#ifndef PINTOS_DEVICE_ID_H
#define PINTOS_DEVICE_ID_H

#include "libk/common.h"

namespace device_id {
enum category : uint32_t { processor_thread, timer, interrupt_controller };

namespace type {

namespace processor_thread {
enum : uint32_t { generic, amd, intel, vmware };
}

namespace timer {
enum : uint32_t { generic, legacy_pit, apic, hpet };
}

namespace interrupt_controller {
enum : uint32_t { generic, legacy_pic, apic, io_apic };
}

} // namespace type
} // namespace device_id
#endif // PINTOS_DEVICE_ID_H
