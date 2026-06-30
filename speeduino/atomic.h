#pragma once
#include "hardware/sync.h"

#if !defined(ATOMIC)
  #if defined(ARDUINO_ARCH_RP2040)
    // Menggunakan Hardware Spinlock khusus nomor 31 (Aman untuk Dual-Core & USB)
    // Spinlock mengunci sinkronisasi data antar core TANPA mematikan interupsi USB CDC
    #define ATOMIC() \
      for (uint32_t _lock = spin_lock_blocking(spin_lock_instance(31)), _done = 0; \
           !_done; \
           spin_unlock(spin_lock_instance(31)), _done = 1)
  #else
    #include <SimplyAtomic.h>
  #endif
#endif
