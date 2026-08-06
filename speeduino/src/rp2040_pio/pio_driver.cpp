#include "../../board_definition.h"

#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include "pio_driver.h"
#include "../../globals.h" // Diperlukan untuk mengakses currentStatus, pinTrigger, dll.

// Array statis internal untuk merekam duration_ticks terakhir (0=Crank, 1=Cam, 2=Cam2)
static volatile uint32_t last_hardware_ticks[3] = {0, 0, 0};

// Fungsi pembaca cepat untuk mengekspos ticks Crank utama ke berkas luar
uint32_t pio_get_last_crank_ticks(void) {
    return last_hardware_ticks[0];
}
void pio_reset_hardware_ticks(void) {
    last_hardware_ticks[0] = 0; // Bersihkan ticks Crank
    last_hardware_ticks[1] = 0; // Bersihkan ticks Cam
    last_hardware_ticks[2] = 0; // Bersihkan ticks Cam2
}
// =============================================================================
// HARDWARE INITIALIZATION DRIVERS
// =============================================================================
void pio_trigger_init(uint8_t pin, uint8_t edge, callback_t callback ) {
    
    uint offset_decoder = pio_add_program(PICOSPEED_PIO0, &picospeed_decoder_program);
    uint offset_cam2    = pio_add_program(PICOSPEED_PIO1, &picospeed_decoder_program);
    
    float trigger_div = (float)clock_get_hz(clk_sys) / DECODER_CLK_HZ;
    if (pin==pinNumbers.pinTrigger2){
    picospeed_decoder_program_init(PICOSPEED_PIO0, PICOSPEED_SM_CAM, offset_decoder, pinNumbers.pinTrigger2, trigger_div);
    }
    else if (pin==pinNumbers.pinTrigger3) {
    picospeed_decoder_program_init(PICOSPEED_PIO1, PICOSPEED_SM_CAM2, offset_cam2, pinNumbers.pinTrigger3, trigger_div);
    }
    else {
    picospeed_decoder_program_init(PICOSPEED_PIO0, PICOSPEED_SM_CRANK, offset_decoder, pinNumbers.pinTrigger, trigger_div);
    }
}
void pio_inj_init(void) {
    uint offset_inj     = pio_add_program(PICOSPEED_PIO0, &inj_output_program);
    uint8_t baseInjectorPin = pinNumbers.injectorPins[0];
    float inj_ign_div = (float)clock_get_hz(clk_sys) / OUTPUT_CLK_HZ; 
    inj_output_program_init(PICOSPEED_PIO0, PICOSPEED_SM_INJ, offset_inj, baseInjectorPin, INJ_CHANNELS, inj_ign_div);
    
}

void pio_ign_init(void) {
    uint offset_ign     = pio_add_program(PICOSPEED_PIO0, &ign_output_program);
    uint8_t baseCoilPin     = pinNumbers.coilPins[0];
    float inj_ign_div = (float)clock_get_hz(clk_sys) / OUTPUT_CLK_HZ;
    ign_output_program_init(PICOSPEED_PIO0, PICOSPEED_SM_IGN, offset_ign, baseCoilPin, IGN_CHANNELS, inj_ign_div);
}

bool pio_decoder_read(uint8_t pin, PioDecoderData &output) {
    PIO target_pio = PICOSPEED_PIO0;
    uint target_sm = PICOSPEED_SM_CRANK;
    uint8_t index = 0;

    if (pin == pinNumbers.pinTrigger2) {
        target_pio = PICOSPEED_PIO0;
        target_sm  = PICOSPEED_SM_CAM;
        index = 1;
    } else if (pin == pinNumbers.pinTrigger3) {
        target_pio = PICOSPEED_PIO1;
        target_sm  = PICOSPEED_SM_CAM2;
        index = 2;
    }

    if (pio_sm_is_rx_fifo_empty(target_pio, target_sm)) {
        return false; 
    }
    
    uint32_t raw_pio_data = pio_sm_get(target_pio, target_sm);
    uint32_t remaining_ticks = raw_pio_data & 0x7FFFFFFF;
    uint32_t total_system_cycles = 0x7FFFFFFF - remaining_ticks;

    // Output duration_ticks murni hasil kalkulasi hardware PIO Anda
    output.duration_ticks = total_system_cycles >> 1;

    // SIMPAN LANGSUNG KE MEMORI STATUS INTERNAL DRIVER
    last_hardware_ticks[index] = output.duration_ticks;

    uint8_t edge_flag = (raw_pio_data >> 31) & 0x01;
    output.edge_type = (edge_flag == 1) ? FALLING : RISING;

    return true; 
}

#endif
