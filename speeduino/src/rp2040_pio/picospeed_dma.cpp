#include "../../board_definition.h"
#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)

#include "picospeed_dma.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h" 
#include "scheduler.h"              

volatile picospeed_engine_buffer_t picospeed_engine_shared = {0};
picospeed_engine_buffer_t core1_engine_local = {0};

int dma_fuel_chan = -1;
int dma_ign_chan = -1;

// Register internal Core 1 untuk menyimpan status bitmask 8-channel terakhir
static uint32_t global_fuel_bitmask = 0x00000000;
static uint32_t global_ign_bitmask  = 0x00000000;

// Menyimpan referensi waktu (timestamp) dari kejadian eksekusi terakhir
static uint32_t last_fuel_event_us = 0;
static uint32_t last_ign_event_us  = 0;


void __not_in_flash_func(dma_handler)() {
    // Cek apakah saluran pengapian yang memicu interupsi selesai transfer
    if (dma_hw->ints0 & (1 << dma_ign_chan)) {
        dma_hw->ints0 = (1 << dma_ign_chan); // Bersihkan flag interupsi
        picospeed_dma_scheduler_loop();      // Panggil loop konsolidator Anda untuk menyuplai data berikutnya
    }
    
    // Cek apakah saluran bahan bakar yang memicu interupsi
    if (dma_hw->ints0 & (1 << dma_fuel_chan)) {
        dma_hw->ints0 = (1 << dma_fuel_chan);
        picospeed_dma_scheduler_loop();
    }
}

// =============================================================================
// ALOKASI FISIK VARIABEL GLOBAL (SHARED MEMORY) LINTAS CORE
// =============================================================================

void picospeed_global_dma_setup(void) {
  // Setup DMA Injektor (Fuel)
  dma_fuel_chan = dma_claim_unused_channel(true);
  dma_channel_config config_fuel = dma_channel_get_default_config(dma_fuel_chan);
  
  channel_config_set_transfer_data_size(&config_fuel, DMA_SIZE_32); 
  channel_config_set_read_increment(&config_fuel, true);            
  channel_config_set_write_increment(&config_fuel, false);          
  channel_config_set_dreq(&config_fuel, pio_get_dreq(pio0, 2, true));
  
  dma_channel_configure(
    dma_fuel_chan, 
    &config_fuel,
    &pio0->txf[2],                  
    NULL,                           
    3,                              
    false                           
  );

  // Setup DMA Pengapian (Ignition)
  dma_ign_chan = dma_claim_unused_channel(true);
  dma_channel_config config_ign = dma_channel_get_default_config(dma_ign_chan);
  
  channel_config_set_transfer_data_size(&config_ign, DMA_SIZE_32);  
  channel_config_set_read_increment(&config_ign, true);             
  channel_config_set_write_increment(&config_ign, false);           
  channel_config_set_dreq(&config_ign, pio_get_dreq(pio0, 3, true));
  
  dma_channel_configure(
    dma_ign_chan, 
    &config_ign,
    &pio0->txf[3],                  
    NULL,                           
    3,                              
    false                           
  );

  last_fuel_event_us = time_us_32();
  last_ign_event_us = time_us_32();

dma_channel_set_irq0_enabled(dma_ign_chan, true);
dma_channel_set_irq0_enabled(dma_fuel_chan, true);

// Daftarkan fungsi dma_handler ke Core yang sedang berjalan (Core 1)
irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
irq_set_enabled(DMA_IRQ_0, true);

// Pemicuan awal (initial trigger) agar ring-buffer pertama kali berjalan
picospeed_dma_scheduler_loop();
}

void picospeed_push_fuel_to_buffer(uint32_t mask, uint32_t delay_us, uint32_t duration_us) {
  ATOMIC() {
    uint8_t next_head = (picospeed_engine_shared.fuel_head + 1) % PICOSPEED_DMA_QUEUE_DEPTH;
    if (next_head != picospeed_engine_shared.fuel_tail) {
        picospeed_engine_shared.fuel_queue[picospeed_engine_shared.fuel_head].mask = mask;
        picospeed_engine_shared.fuel_queue[picospeed_engine_shared.fuel_head].delay_ticks = delay_us; 
        picospeed_engine_shared.fuel_queue[picospeed_engine_shared.fuel_head].duration_ticks = duration_us; 
        
        __dmb(); // Data Memory Barrier untuk sinkronisasi ARM dual-core
        picospeed_engine_shared.fuel_head = next_head;
    }
  }
}

void picospeed_push_ign_to_buffer(uint32_t mask, uint32_t delay_us, uint32_t duration_us) {
  ATOMIC() {
    uint8_t next_head = (picospeed_engine_shared.ign_head + 1) % PICOSPEED_DMA_QUEUE_DEPTH;
    if (next_head != picospeed_engine_shared.ign_tail) {
        picospeed_engine_shared.ign_queue[picospeed_engine_shared.ign_head].mask = mask;
        picospeed_engine_shared.ign_queue[picospeed_engine_shared.ign_head].delay_ticks = delay_us;
        picospeed_engine_shared.ign_queue[picospeed_engine_shared.ign_head].duration_ticks = duration_us; 
        
        __dmb(); 
        picospeed_engine_shared.ign_head = next_head;
    }
  }
}

void picospeed_dma_scheduler_loop(void) {
    // -------------------------------------------------------------------------
    // KONSOLIDATOR SUB-SISTEM INJEKTOR (FUEL)
    // -------------------------------------------------------------------------
    uint8_t local_fuel_head = picospeed_engine_shared.fuel_head;
    uint8_t local_fuel_tail = picospeed_engine_shared.fuel_tail;

    if (!dma_channel_is_busy(dma_fuel_chan) && (local_fuel_head != local_fuel_tail)) {
        uint32_t ev_mask     = picospeed_engine_shared.fuel_queue[local_fuel_tail].mask;
        uint32_t ev_delay    = picospeed_engine_shared.fuel_queue[local_fuel_tail].delay_ticks;
        uint32_t ev_duration = picospeed_engine_shared.fuel_queue[local_fuel_tail].duration_ticks;
        
        if (ev_duration == 1) {
            global_fuel_bitmask |= ev_mask;   
        } else {
            global_fuel_bitmask &= ~ev_mask;  
        }

       uint32_t delta_us = ev_delay - last_fuel_event_us;
        
        // FIX PERBAIKAN: Berikan indeks [0] karena propertinya adalah Array
        core1_engine_local.fuel_queue[0].mask = global_fuel_bitmask;
        core1_engine_local.fuel_queue[0].delay_ticks = delta_us * 20; 
        core1_engine_local.fuel_queue[0].duration_ticks = 0; 

        // Transfer 3 Word utuh dari alamat elemen pertama memori lokal
        dma_channel_transfer_from_buffer_now(dma_fuel_chan, &core1_engine_local.fuel_queue[0], 3);
        
        last_fuel_event_us = ev_delay;  
        
        __dmb();
        picospeed_engine_shared.fuel_tail = (local_fuel_tail + 1) % PICOSPEED_DMA_QUEUE_DEPTH;
    }

    // -------------------------------------------------------------------------
    // KONSOLIDATOR SUB-SISTEM PENGAPIAN (IGNITION)
    // -------------------------------------------------------------------------
    uint8_t local_ign_head = picospeed_engine_shared.ign_head;
    uint8_t local_ign_tail = picospeed_engine_shared.ign_tail;

    if (!dma_channel_is_busy(dma_ign_chan) && (local_ign_head != local_ign_tail)) {
        uint32_t ev_mask     = picospeed_engine_shared.ign_queue[local_ign_tail].mask;
        uint32_t ev_delay    = picospeed_engine_shared.ign_queue[local_ign_tail].delay_ticks;
        uint32_t ev_duration = picospeed_engine_shared.ign_queue[local_ign_tail].duration_ticks;
        
        if (ev_duration == 1) {
            global_ign_bitmask |= ev_mask;
        } else {
            global_ign_bitmask &= ~ev_mask;
        }

        uint32_t delta_us = ev_delay - last_ign_event_us;
        
        core1_engine_local.ign_queue[0].mask = global_ign_bitmask;
        core1_engine_local.ign_queue[0].delay_ticks = delta_us * 20;
        core1_engine_local.ign_queue[0].duration_ticks = 0;

        // Semburkan 3 Word terpadu ke register DMA menuju PIO FIFO
        dma_channel_transfer_from_buffer_now(dma_ign_chan, &core1_engine_local.ign_queue[0], 3);
        
        last_ign_event_us = ev_delay;
        
        __dmb(); // Pembatas memori dual-core
        picospeed_engine_shared.ign_tail = (local_ign_tail + 1) % PICOSPEED_DMA_QUEUE_DEPTH;
    }
}

volatile uint16_t picospeed_fuel_registers[8] = {0};
volatile uint16_t picospeed_ign_registers[8]  = {0};

void picospeed_capture_dynamic_adjust(volatile void *compare_ptr, uint16_t new_val) {
    uint32_t channel_mask = 0;
    bool is_ignition = false;
    uintptr_t addr = (uintptr_t)compare_ptr;

    // 1. FILTER EKSKLUSIF FUEL: Hanya terima alamat dari indeks 0 sampai 7
    uintptr_t fuel_start = (uintptr_t)&picospeed_fuel_registers[0];
    uintptr_t fuel_end   = (uintptr_t)&picospeed_fuel_registers[7];
    
    if (addr >= fuel_start && addr <= fuel_end) {
        int8_t fuel_index = (addr - fuel_start) / sizeof(uint16_t);
        if (fuel_index >= 0 && fuel_index < 8) {
            channel_mask = (1 << fuel_index);
            is_ignition = false;
        }
    }
    // 2. FILTER EKSKLUSIF IGNITION: Hanya terima alamat dari indeks 0 sampai 7
    else {
        uintptr_t ign_start = (uintptr_t)&picospeed_ign_registers[0];
        uintptr_t ign_end   = (uintptr_t)&picospeed_ign_registers[7];
        
        if (addr >= ign_start && addr <= ign_end) {
            int8_t ign_index = (addr - ign_start) / sizeof(uint16_t);
            if (ign_index >= 0 && ign_index < 8) {
                channel_mask = (1 << ign_index);
                is_ignition = true;
            }
        }
    }

    // 3. EKSEKUSI REVISI TIMING (VVT, BOOST, IDLE, FAN AKAN KELUAR DI SINI KARENA MASK = 0)
    if (channel_mask != 0) {
        if (is_ignition) {
            picospeed_push_ign_to_buffer(channel_mask, (uint32_t)new_val, 2); // 2 = REVISI
        } else {
            picospeed_push_fuel_to_buffer(channel_mask, (uint32_t)new_val, 2);
        }
    }
}

#endif
