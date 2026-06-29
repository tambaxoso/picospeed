#include <Arduino.h>
#include "engine_rp2040.h"
#include "globals.h"
#include "hardware/pio.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "engine_core.pio.h"

// Variabel Kontrol Sinkronisasi (Gunakan Volatile karena antar-Core)
volatile int16_t crank_degree = 0;
volatile bool is_synced = false;
uint32_t last_tooth_ticks = 0;

// Konstanta RPM untuk 36-1 (Clock 133MHz)
// Rumus: (60 * 133.000.000) / 36 gigi
const uint32_t RPM_CONSTANT_36_1 = 221666666;

/**
 * @brief Fungsi Inisialisasi State Machine PIO
 */
void engine_core_program_init(PIO pio, uint sm, uint offset, uint entry_offset, uint pin_base, uint pin_count, bool is_output) {
  pio_sm_config c = engine_core_program_get_default_config(offset);

  if(is_output) {
    for (uint i = 0; i < pin_count; i++) { pio_gpio_init(pio, pin_base + i); }
    pio_sm_set_consecutive_pindirs(pio, sm, pin_base, pin_count, true);
    sm_config_set_out_pins(&c, pin_base, pin_count);
    sm_config_set_out_shift(&c, true, false, 32); // Shift Right (LSB)
  } else {
    pio_gpio_init(pio, pin_base);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_base, 1, false);
    sm_config_set_in_pins(&c, pin_base);
    sm_config_set_jmp_pin(&c, pin_base);
  }

  pio_sm_init(pio, sm, offset + entry_offset, &c);
}


/**
 * @brief PENGIRIM DATA KE PIO (Inj/Ign)
 * format paket: [ DURATION (28 bit) | MASK (4 bit) ]
 */
void send_engine_output(uint sm, uint8_t mask_bits, uint32_t duration_us) {
  uint32_t duration_ticks = duration_us * 133;
  uint32_t packet = (duration_ticks << 4) | (mask_bits & 0xF);
  pio_sm_put_blocking(pio0, sm, packet);
}

/**
 * @brief LOOP UTAMA CORE 1 (Execution Engine)
 * Fokus pada pembacaan PIO dan sinkronisasi real-time
 */
void core1_entry() {
  PIO pio = pio0;
  
  while (1) {
    // 1. Cek data gigi kruk as dari PIO FIFO
    if (!pio_sm_is_rx_fifo_empty(pio, 0)) {
      uint32_t raw_x = pio_sm_get(pio, 0);
      uint32_t current_ticks = 0xFFFFFFFF - raw_x;

      // 2. Kalkulasi RPM
      if (current_ticks > 50) {
        uint32_t calc_rpm = RPM_CONSTANT_36_1 / current_ticks;
        if (calc_rpm < 15000) { currentStatus.RPM = (uint16_t)calc_rpm; }
      }

      // 3. Cek Sinkronisasi (Gap Detection 1.5x)
      if (current_ticks > (last_tooth_ticks * 1.5) && last_tooth_ticks > 0) {
        crank_degree = 0; // Missing tooth ditemukan
        is_synced = true;
      } else {
        crank_degree += 10; // Setiap gigi 36-1 adalah 10 derajat
        if (crank_degree >= 360) { is_synced = false; }
      }
      last_tooth_ticks = current_ticks;

      // 4. Contoh Pemicuan Output (Injeksi)
      // Jika derajat sudah sesuai target, tembak injektor
      // send_engine_output(2, 0b0001, currentStatus.PW1); 
    }
  }
}

/**
 * @brief SETUP GLOBAL PIO (Dipanggil dari initBoard)
 */
void setup_all_engine_pio() {
  PIO pio = pio0;
  uint offset = pio_add_program(pio, &engine_core_program);

  // SM0: Decoder Crank (GP0)
  engine_core_program_init(pio, 0, offset, engine_core_offset_entry_crank, 0, 1, false);

  // SM2: Injection (GP2-GP5)
  engine_core_program_init(pio, 2, offset, engine_core_offset_entry_output, 2, 4, true);

  // SM3: Ignition (GP6-GP9)
  engine_core_program_init(pio, 3, offset, engine_core_offset_entry_output, 6, 4, true);

  // Jalankan semua SM secara sinkron
  pio_enable_sm_mask_in_sync(pio, (1u << 0) | (1u << 2) | (1u << 3));
  
  // Luncurkan Core 1
  multicore_launch_core1(core1_entry);
}

/*
// Channel 1 adalah LSB (bit paling kanan)
const uint32_t output_masks = {
  0b0000, // Matikan semua
  0b0001, // Channel 1 (GP2 / GP6)
  0b0010, // Channel 2 (GP3 / GP7)
  0b0100, // Channel 3 (GP4 / GP8)
  0b1000, // Channel 4 (GP5 / GP9)
  0b1111  // Nyalakan semua
};
*/
/*
Pola Gigi  Total Teeth (Ekuivalen)   Konstanta (133MHz)     Keterangan
36-1                36                221.666.666           Standar Ford/Aftermarket
60-2                60                133.000.000           Standar Bosch/Eropa
24-1                24                332.500.000           Standar Toyota/Denso
12-1                12                665.000.000           Mesin kecil / Motor
4-1                 4                 1.995.000.000         Distribusi standar
*/


#define ENGINE_SYNC_LOCK_ID 17
#define MSG_SYNC_LOST 0xEE
#define MSG_SYNC_GAINED 0xAA

volatile bool engine_synced = false;
volatile uint32_t sync_loss_count = 0;
spin_lock_t *sync_lock;

void setup_sync_system() {
  sync_lock = spin_lock_init(ENGINE_SYNC_LOCK_ID);
}

/**
 * @brief Fungsi update data mesin yang aman menggunakan Hardware Spinlock
 */
void update_engine_data_safe(uint16_t calculated_rpm, uint8_t local_sync_errors) {
  uint32_t flags = spin_lock_blocking(sync_lock);
  currentStatus.RPM = calculated_rpm;
  currentStatus.syncLossCounter = local_sync_errors;
  spin_unlock(sync_lock, flags);
}

void core1_check_logic(uint32_t tooth_count) {
  bool error_detected = false;

  if (tooth_count != 35) { 
    error_detected = true;
  }

  if (error_detected) {
    uint32_t flags = spin_lock_blocking(sync_lock);
    engine_synced = false;
    sync_loss_count++;
    spin_unlock(sync_lock, flags);

    multicore_fifo_push_blocking(MSG_SYNC_LOST);
  }
}

void core0_monitor_sync() {
  if (multicore_fifo_rvalid()) {
    uint32_t msg = multicore_fifo_pop_blocking();

    if (msg == MSG_SYNC_LOST) {
      // Sesuai dengan standar penulisan register status Speeduino asli
      currentStatus.syncLossCounter++;
    } 
    else if (msg == MSG_SYNC_GAINED) {
      // Handler saat mendapatkan sinkronisasi kembali
    }
  }
}