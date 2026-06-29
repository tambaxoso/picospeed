#pragma once
#include "port_pin.h"
#include "../rp2040_io/picospeed_io.h"
/** 
 * @brief Sebuah class untuk operasi input pin yang lebih cepat dari digitalRead().
 * Di RP2040, ini membaca register input SIO secara atomik dalam 1 siklus clock.
 */
class fastInputPin_t 
{
public:
  /** @brief Inisialisasi pin input */
  void setPin(uint8_t pin, uint8_t mode = INPUT);

  /** @brief Cek apakah pin bernilai HIGH */
  bool isPinHigh(void) const;

  /** @brief Cek apakah pin bernilai LOW */
  bool isPinLow(void) const {
    return !isPinHigh();
  }

  /** @brief Cek apakah status nomor pin valid */
  bool isValid(void) const {
    return _port_pin.isValid();
  }

private:
  port_pin_t _port_pin;
};
