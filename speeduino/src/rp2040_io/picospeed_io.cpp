#include "picospeed_io.h"

#if defined(ARDUINO_ARCH_RP2040)

void PicospeedPin::setPin(uint8_t pin, uint8_t mode) {
  if (pin != 255) {
    _pin = pin;
    _is_pwm_active = false;
    pinMode(pin, mode);
  }
}

void PicospeedPin::setPinHigh(void) {
  if (isValid()) {
    if (_is_pwm_active) {
      // Jika pin sedang di mode PWM, paksa ke Duty 100% (Wrap 10000)
      pwm_set_gpio_level(_pin, 10000);
    } else {
      // SIO Hardware Atomic 1-Cycle write (Bebas dari ATOMIC software / USB Aman)
      sio_hw->gpio_set = (1ul << _pin);
    }
  }
}

void PicospeedPin::setPinLow(void) {
  if (isValid()) {
    if (_is_pwm_active) {
      // Jika pin sedang di mode PWM, paksa ke Duty 0%
      pwm_set_gpio_level(_pin, 0);
    } else {
      // SIO Hardware Atomic 1-Cycle write
      sio_hw->gpio_clr = (1ul << _pin);
    }
  }
}

bool PicospeedPin::read(void) const {
  if (isValid()) {
    // Membaca input digital dalam 1 siklus clock prosesor
    return (sio_hw->gpio_in & (1ul << _pin)) != 0;
  }
  return false;
}

void PicospeedPin::writePWM(uint8_t duty_255) {
  if (isValid()) {
    // Runtime Hijacking: Aktifkan hardware PWM jika fungsi ini dipanggil
    if (!_is_pwm_active) {
      gpio_set_function(_pin, GPIO_FUNC_PWM);
      uint slice_num = pwm_gpio_to_slice_num(_pin);
      
      // Frekuensi dasar ECU ~100Hz (133MHz / 133 = 1MHz, Wrap 10000 = 100Hz)
      pwm_set_clkdiv(slice_num, 133.0f);
      pwm_set_wrap(slice_num, 10000); 
      pwm_set_enabled(slice_num, true);
      
      _is_pwm_active = true;
    }

    // Konversi skala 0-255 Speeduino ke skala 0-10000 hardware secara cepat
    uint32_t target_level = (uint32_t)duty_255 * 10000 / 255;
    pwm_set_gpio_level(_pin, target_level);
  }
}

#endif
