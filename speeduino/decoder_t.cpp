#include <Arduino.h>
#include "decoder_t.h"
#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include "src/rp2040_pio/pio_driver.h"
#endif
#pragma GCC optimize("Os")

// Just in case
static_assert(TRIGGER_EDGE_NONE != LOW, "LOW edge value conflict");
static_assert(TRIGGER_EDGE_NONE != HIGH, "HIGH edge value conflict");
static_assert(TRIGGER_EDGE_NONE != RISING, "RISING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != FALLING, "FALLING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != CHANGE, "CHANGE edge value conflict");

uint8_t interrupt_t::attach(uint8_t pin)
{
    detach(pin);

    _pin.setPin(pin);
    if (isValid())
    {
        #if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
        pio_trigger_init(pin, edge, callback);
        #else
        attachInterrupt(digitalPinToInterrupt(pin), callback, edge);
        #endif
        return pin;
    }
    return NOT_A_PIN;
}  

/** @brief Detach the interrupt from a pin */
void interrupt_t::detach(uint8_t pin)
{
    #if defined(ARDUINO_ARCH_RP2040)
    //
    #else
    detachInterrupt( digitalPinToInterrupt(pin) );
    #endif
    _pin.setPin(NOT_A_PIN);
}  

bool interrupt_t::isTriggered(void) const
{
    return isValid()
    && (
           (edge==CHANGE)
        || (edge==FALLING && !isPinHigh())
        || (edge==RISING && isPinHigh())
    );
}

bool interrupt_t::isPinHigh(void) const
{
    #if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
    // 1. Tentukan nomor pin fisik secara dinamis lewat alamat memori objek (this)
    uint8_t pin = pinNumbers.pinTrigger; // Default Primary
    const interrupt_t* primary_ptr = this;
    const interrupt_t* secondary_ptr = primary_ptr + 1;
    const interrupt_t* tertiary_ptr = primary_ptr + 2;

    if (this == secondary_ptr) {
        pin = pinNumbers.pinTrigger2;
    } else if (this == tertiary_ptr) {
        pin = pinNumbers.pinTrigger3;
    }

    // 2. Gunakan fungsi pio_decoder_read untuk membedah data FIFO dari hardware
    PioDecoderData data;
    if (pio_decoder_read(pin, data)) {
        // Jika edge terakhir yang ditangkap PIO adalah FALLING (sinyal baru saja jatuh ke LOW),
        // itu berarti kondisi pin saat ini sudah berada di posisi LOW (return false).
        // Sebaliknya, jika RISING (baru saja naik ke HIGH), berarti pin saat ini HIGH (return true).
        return (data.edge_type == RISING);
    }
    
    // Failsafe: Jika FIFO kosong, gunakan pembacaan hardware_pin bawaan fastInputPin sebagai cadangan
    return _pin.isPinHigh();
    #else
    // Fallback bawaan asli untuk platform non-RP2040 (AVR Mega, dll)
    return _pin.isPinHigh();
    #endif
}