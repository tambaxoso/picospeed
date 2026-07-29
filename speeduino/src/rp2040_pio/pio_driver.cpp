#include "../../board_definition.h"

#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include "pio_driver.h"
#include "../../decoders.h"
#include "../../globals.h" // Diperlukan untuk mengakses currentStatus, pinTrigger, dll.

TriggerShadowState primary_shadow   = {false, false, 99, 0};
TriggerShadowState secondary_shadow = {false, false, 99, 0};
TriggerShadowState tertiary_shadow  = {false, false, 99, 0};

// =============================================================================
// HARDWARE INITIALIZATION DRIVERS
// =============================================================================
void pio_driver_init(void) {
    uint offset_decoder = pio_add_program(PICOSPEED_PIO0, &picospeed_decoder_program);
    uint offset_inj     = pio_add_program(PICOSPEED_PIO0, &inj_output_program);
    uint offset_ign     = pio_add_program(PICOSPEED_PIO0, &ign_output_program);
    uint offset_cam2    = pio_add_program(PICOSPEED_PIO1, &picospeed_decoder_program);
    uint8_t baseInjectorPin = pinNumbers.injectorPins[0];
    uint8_t baseCoilPin     = pinNumbers.coilPins[0];
    float trigger_div = (float)clock_get_hz(clk_sys) / DECODER_CLK_HZ;
    float inj_ign_div = (float)clock_get_hz(clk_sys) / OUTPUT_CLK_HZ; 

    picospeed_decoder_program_init(PICOSPEED_PIO0, PICOSPEED_SM_CRANK, offset_decoder, pinNumbers.pinTrigger, trigger_div);
    picospeed_decoder_program_init(PICOSPEED_PIO0, PICOSPEED_SM_CAM, offset_decoder, pinNumbers.pinTrigger2, trigger_div);
    if (pinNumbers.pinTrigger3 != NOT_A_PIN) {
    picospeed_decoder_program_init(PICOSPEED_PIO1, PICOSPEED_SM_CAM2, offset_cam2, pinNumbers.pinTrigger3, trigger_div);
    }
    
    inj_output_program_init(PICOSPEED_PIO0, PICOSPEED_SM_INJ, offset_inj, baseInjectorPin, INJ_CHANNELS, inj_ign_div);
    ign_output_program_init(PICOSPEED_PIO0, PICOSPEED_SM_IGN, offset_ign, baseCoilPin, IGN_CHANNELS, inj_ign_div);
}

bool pio_decoder_read(PIO pio, uint sm, PioDecoderData &output) {
    // 1. Ambil data 32-bit mentah dari FIFO PIO (Akan memblokir CPU jika FIFO kosong)
    uint32_t raw_pio_data = pio_sm_get_blocking(pio, sm);

    // 2. Ekstrak 31 bit bawah (Sisa nilai down-counter stopwatch X)
    uint32_t remaining_ticks = raw_pio_data & 0x7FFFFFFF;
    
    // Kalkulasi jumlah total siklus sistem yang terlewati
    uint32_t total_system_cycles = 0xFFFFFFFF - remaining_ticks;

    // KOREKSI UTAMA: Dibagi 2 karena loop PIO membutuhkan 2 cycle per pengurangan X
    // Menggunakan bitwise shift >> 1 agar eksekusi secepat kilat di Core 1
    output.duration_ticks = total_system_cycles >> 1;

    // 3. Ekstrak dan konversi penanda Edge (Bit 31)
    // Ingat logika PIO Assembly Anda:
    // set y, 1 dikirim saat masa HIGH berakhir (Sinyal jatuh ke LOW / FALLING)
    // set y, 0 dikirim saat masa LOW berakhir (Sinyal naik ke HIGH / RISING)
    uint8_t edge_flag = (raw_pio_data >> 31) & 0x01;
    if (edge_flag == 1) {
        output.edge_type = FALLING;
    } else {
        output.edge_type = RISING;
    }

    // WAJIB RETURN TRUE: Menandakan data FIFO berhasil diekstrak dengan sukses
    return true; 
}

#endif
