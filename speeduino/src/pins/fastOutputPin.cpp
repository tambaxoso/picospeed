#include "fastOutputPin.h"
#include "../../atomic.h"
#include "../../board_definition.h"


// LCOV_EXCL_START

/* Fallback untuk AVR / Mega 2560 */
void fastOutputPin_t::setPin(uint8_t pin, uint8_t mode) {
  if (pin != NOT_A_PIN) {
    pinMode(pin, mode);
    _port_pin.port = portOutputRegister(digitalPinToPort(pin));
    _port_pin.mask = digitalPinToBitMask(pin);
  }
}

void fastOutputPin_t::setPinHigh(void) {
  if (isValid()) {
    ATOMIC() { *_port_pin.port |= _port_pin.mask; }
  }
}

void fastOutputPin_t::setPinLow(void) {
  if (isValid()) {
    ATOMIC() { *_port_pin.port &= ~_port_pin.mask; }
  }
}


// LCOV_EXCL_STOP
