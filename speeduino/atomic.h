#pragma once

#include "board_definition.h"

#if !defined(ATOMIC)

  #if defined(ARDUINO_ARCH_RP2040) || defined(PICO_BOARD)
    // Core hardware integration for the Raspberry Pi Pico (RP2040)
    #include "hardware/sync.h"

    /**
     * @brief Emulates an atomic block safely on a single RP2040 core.
     * Uses a single-execution loop pattern to dynamically save, disable,
     * and restore interrupt states in a clean macro block structural layout.
     */
    #define ATOMIC() \
      for (uint32_t _saved_interrupts = save_and_disable_interrupts(), _done = 0; \
           !_done; \
           restore_interrupts(_saved_interrupts), _done = 1)

  #else
    // Fallback block for legacy 8-bit AVR or other officially supported architectures
    #include <SimplyAtomic.h>
  #endif

#endif
