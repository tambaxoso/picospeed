#include <Arduino.h>
#include "globals.h"

void engine_core_program_init(PIO pio, uint sm, uint offset, uint entry_offset, uint pin_base, uint pin_count, bool is_output);
void send_engine_output(uint sm, uint8_t mask_bits, uint32_t duration_us);
void core1_entry();
void setup_all_engine_pio();
void setup_sync_system();
void update_engine_data_safe(uint16_t calculated_rpm, uint8_t local_sync_errors);
void core1_check_logic(uint32_t tooth_count);
void core0_monitor_sync();