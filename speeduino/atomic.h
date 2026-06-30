#pragma once


#if !defined(ATOMIC)
  #if defined(ARDUINO_ARCH_RP2040)
    #include "hardware/sync.h"
    // Menggunakan Hardware Spinlock khusus nomor 31 (Aman untuk Dual-Core & USB)
    // Spinlock mengunci sinkronisasi data antar core TANPA mematikan interupsi USB CDC
    #define ATOMIC() \
      for (uint32_t _lock = spin_lock_blocking(spin_lock_instance(31)), _done = 0; \
           !_done; \
           spin_unlock(spin_lock_instance(31)), _done = 1)
    #elif defined(UNIT_TEST) || defined(NATIVE)
    // Solusi Khusus Unit Test PC / YAML Simulation:
    // Membuat macro kosong (Stub) agar kode tetap bisa di-build oleh PC
    #define ATOMIC() for (int _done = 0; !_done; _done = 1)
  #else
    #include <SimplyAtomic.h>
  #endif
#endif
