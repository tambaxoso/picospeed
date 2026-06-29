#include "pinMapping.h"
#include "../../board_definition.h"
#include "../../preprocessor.h"

uint8_t pinTranslate(uint8_t rawPin) {
  uint8_t outputPin = rawPin;

#if defined(ARDUINO_ARCH_RP2040)
  if (rawPin > BOARD_MAX_DIGITAL_PINS) {
    uint8_t analogOffset = rawPin - BOARD_MAX_DIGITAL_PINS - 1;
    if (analogOffset <= 3) {
      outputPin = A0 + analogOffset;
    } else {
      outputPin = 255;
    }
  }
#else
  if (rawPin > BOARD_MAX_DIGITAL_PINS) {
    outputPin = A8 + (outputPin - BOARD_MAX_DIGITAL_PINS - 1);
  }
#endif

  return outputPin;
}

uint8_t pinTranslateAnalog(uint8_t rawPin) {
#if defined(ARDUINO_ARCH_RP2040)
  static const uint8_t RP2040_ANALOG_PINS[] = { A0, A1, A2, A3 };
  if (rawPin < 4) {
    return RP2040_ANALOG_PINS[rawPin];
  }
  return 255;
#else
  if (rawPin < _countof(ANALOG_PINS)) {
    return ANALOG_PINS[rawPin];
  }
  return rawPin;
#endif
}
