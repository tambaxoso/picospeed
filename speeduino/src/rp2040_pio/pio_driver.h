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

using callback_t = void(*)(void);

struct PioDecoderData {
    uint32_t duration_ticks;
    uint8_t edge_type;
};
void pio_trigger_init(uint8_t pin, uint8_t edge, callback_t callback );
bool pio_decoder_read(uint8_t pin, PioDecoderData &output);
void pio_inj_init(void);
void pio_ign_init(void);

#endif