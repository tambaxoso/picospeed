#pragma once

#include "../../board_definition.h"

#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico_engine.pio.h"
#include "../../decoders.h"
#include "../../decoder_t.h"
#include "../../globals.h"

#define PICOSPEED_PIO0         pio0
#define PICOSPEED_SM_CRANK    0
#define PICOSPEED_SM_CAM      1
#define PICOSPEED_PIO1         pio1
#define PICOSPEED_SM_CAM2     0
#define PICOSPEED_SM_INJ      2
#define SM_INJ_BASE           2
#define PICOSPEED_SM_IGN      3
#define SM_IGN_BASE           (SM_INJ_BASE + INJ_CHANNELS)
#define DECODER_CLK_HZ        100000000.0f
#define OUTPUT_CLK_HZ         200000000.0f

struct PioDecoderData {
    uint32_t duration_ticks; // Durasi pulsa (elapsed time)
    uint8_t edge_type;       // Mengembalikan makro FALLING atau RISING
};
struct TriggerShadowState {
    volatile bool is_high;
    volatile bool is_triggered;
    volatile uint8_t detected_edge;
    volatile uint32_t last_period_us;
};

void pio_driver_init(void);
bool pio_decoder_read(PIO pio, uint sm, PioDecoderData &output);

#endif