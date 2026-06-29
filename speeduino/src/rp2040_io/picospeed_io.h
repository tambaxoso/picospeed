#pragma once
#include <Arduino.h>

#if defined(ARDUINO_ARCH_RP2040)
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/regs/sio.h"

class PicospeedPin 
{
public:
  // Inisialisasi awal pin
  void setPin(uint8_t pin, uint8_t mode = OUTPUT);
  // Fungsi On/Off Digital Kilat (SIO Hardware Atomic)
  void setPinHigh(void);
  void setPinLow(void);
  // Fungsi Membaca Sinyal Input (Crank/Cam/Relay)
  bool read(void) const;
  bool isPinHigh(void) const { return read(); }
  bool isPinLow(void) const  { return !read(); }

  // Pembajak Bitbang Otomatis skala 0-255 ke Hardware PWM
  void writePWM(uint8_t duty_255);

  // Cek validitas pin
  bool isValid(void) const { return _pin != 255; }

private:
  uint8_t _pin = 255;
  bool _is_pwm_active = false;
};
#endif
