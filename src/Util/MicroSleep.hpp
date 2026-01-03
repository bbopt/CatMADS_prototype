/**
 \file   MicroSleep.hpp
 \brief  Portable usleep function (headers)
 \author Wim Lavrijsen
 \date   2021-03-24
 \see    MicroSleep.hpp
 */
#ifndef __NOMAD_4_5_MICROSLEEP__
#define __NOMAD_4_5_MICROSLEEP__

#ifdef _MSC_VER
#include <chrono>
#include <thread>

static inline void usleep(uint64_t usec) {
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

#else
#include <unistd.h>
#endif

#endif // __NOMAD_4_5_MICROSLEEP__
