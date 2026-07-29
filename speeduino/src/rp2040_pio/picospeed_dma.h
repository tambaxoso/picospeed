#pragma once

#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include <stdint.h>
#include "hardware/pio.h"

// Kedalaman antrean di RAM untuk menimbun data sekuensial (Harus kelipatan biner: 8, 16, 32)
#define PICOSPEED_DMA_QUEUE_DEPTH 16

// Paket data 3 Word murni untuk memuaskan skema instruksi pull-block PIO
typedef struct {
    uint32_t mask;
    uint32_t delay_ticks;
    uint32_t duration_ticks;
} dma_packet_t;

typedef struct {
    dma_packet_t fuel_queue[PICOSPEED_DMA_QUEUE_DEPTH];
    dma_packet_t ign_queue[PICOSPEED_DMA_QUEUE_DEPTH];
    volatile uint8_t fuel_head; // Wajib volatile untuk menjamin sinkronisasi antar core
    volatile uint8_t fuel_tail; // Wajib volatile
    volatile uint8_t ign_head;  // Wajib volatile
    volatile uint8_t ign_tail;  // Wajib volatile
} __attribute__((aligned(4))) picospeed_engine_buffer_t;

// =============================================================================
// DEKLARASI VARIABEL GLOBAL (SHARED MEMORY)
// =============================================================================
extern volatile picospeed_engine_buffer_t picospeed_engine_shared;
extern picospeed_engine_buffer_t core1_engine_local;

extern int dma_fuel_chan;
extern int dma_ign_chan;

// =============================================================================
// PROTOTIPE FUNGSI DRIVER
// =============================================================================
void picospeed_global_dma_setup(void);
void picospeed_push_fuel_to_buffer(uint32_t mask, uint32_t delay_us, uint32_t duration_us);
void picospeed_push_ign_to_buffer(uint32_t mask, uint32_t delay_us, uint32_t duration_us);
void picospeed_dma_scheduler_loop(void);
void __not_in_flash_func(dma_handler)(void);
void picospeed_capture_dynamic_adjust(void *compare_ptr, uint16_t new_val);

#endif
