/**
 * @file common_region.cpp
 * @author Shane Menzies
 * @brief
 * @date 2/12/23
 *
 *
 */

#include "common_region.h"

#include "threading/process_def.h"

namespace common_region {

size_t const commmon_region_size = sizeof(threading::process);

}
