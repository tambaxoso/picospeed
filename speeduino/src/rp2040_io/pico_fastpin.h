#pragma once

#include <Arduino.h>
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

/** 
 * @brief A class for output pin operations that is faster than standard Arduino digitalWrite() 
 * 
 * Call setPin() to initialize the pin, then call setPinHigh() to set the pin high.
 */
class pico_OutputPin_t 
{
public:
  pico_OutputPin_t();

  void setPin(uint8_t pin, uint8_t mode = OUTPUT);

  void writePWM(uint8_t duty_255, uint32_t target_freq = 200, bool invert = false);
  void disablePWM(void);

  inline bool isValid(void) const __attribute__((always_inline)) {
    return _pin != 255;
  }

  inline void setPinHigh(void) __attribute__((always_inline)) {
    if (isValid()) {
      if (__builtin_expect(_is_pwm_active, 0)) {
        pwm_set_chan_level(_slice, _chan, 10001); 
        } else {
        digitalWriteFast(_pin, HIGH); 
      }
    }
  }

  inline void setPinLow(void) __attribute__((always_inline)) {
    if (isValid()) {
      if (__builtin_expect(_is_pwm_active, 0)) {
        pwm_set_chan_level(_slice, _chan, 0); 
        } else {
        digitalWriteFast(_pin, LOW); 
      }
    }
  }

private:
  uint8_t  _pin;
  uint8_t  _slice;
  uint8_t  _chan;
  bool     _is_pwm_active;
  uint32_t _current_freq; 
};

/** 
 * @brief A class for input pin operations. 
 * 
 * Call setPin() to initialize the pin, then call isPinHigh() to check if the pin is set high.
 * 
 * @note Must have same signature as fastInputPin_t to allow for interchangeable use.
 */
class pico_InputPin_t {
public:
    pico_InputPin_t() : _pin(255) {}

    void setPin(uint8_t pin, uint8_t mode = INPUT);

    inline bool isValid(void) const __attribute__((always_inline)) {
        return _pin != 255;
    }

    inline bool read(void) const __attribute__((always_inline)) {
        if (isValid()) {
            return digitalReadFast(_pin) != 0;
        }
        return false;
    }

    inline bool isPinHigh(void) const __attribute__((always_inline)) {
        return read(); 
    }

private:
    uint8_t _pin;
};
