#pragma once

#if defined(ARDUINO_ARCH_RP2040)
#include <Arduino.h>
#include <stdint.h>
#include <array>
#include <limits>
#include "hardware/sync.h"
#include "src/rp2040_pio/picospeed_dma.h"
/*
***********************************************************************************************************
* General
*/
#define PICOSPEED_MAPPING
#define TS_SERIAL_BUFFER_SIZE 517 //Size of the serial buffer used by new comms protocol. For SD transfers this must be at least 512 + 1 (flag) + 4 (sector)
#define FPU_MAX_SIZE 0 //Size of the FPU buffer. 0 means no FPU.
#define BOARD_MAX_IO_PINS  30 //digital pins + analog channels + 1
#define BOARD_MAX_DIGITAL_PINS 30 //Pretty sure this isn't right
// EEPROM
#define EEPROM_LIB_H <EEPROM.h>
class EEPROMClass;
using EEPROM_t = EEPROMClass;

// Forbidden pins like USB
static inline bool pinIsReserved(uint8_t pin) { 
  return (pin==23U || pin==24U || pin==25U); 
} 
using COMPARE_TYPE = uint32_t;
  static inline constexpr uint32_t ticksToMicros(COMPARE_TYPE ticks)
{
  return (uint32_t)ticks;
}
static inline constexpr COMPARE_TYPE microsToTicks(uint32_t us)
{
  return (COMPARE_TYPE)us;
}

#define uS_TO_TIMER_COMPARE(uS) ((COMPARE_TYPE)(uS))

#define A4  255
#define A5  255
#define A6  255
#define A7  255
#define A8  255
#define A9  255
#define A13  255
#define A15  255

class Rp2040AtomicGuard {
    private:
        uint32_t status;
    public:
        // Mematikan interupsi lokal pada core saat ini dengan aman menggunakan SDK resmi Pico
        Rp2040AtomicGuard() { status = save_and_disable_interrupts(); }
        // Mengembalikan status interupsi saat blok kode selesai dieksekusi
        ~Rp2040AtomicGuard() { restore_interrupts(status); }
        operator bool() const { return true; }
};

// Ganti atau override makro bawaan Speeduino agar menggunakan Guard RP2040 Anda
#define ATOMIC() if (Rp2040AtomicGuard guard = Rp2040AtomicGuard())

/*
***********************************************************************************************************
* Schedules
*/ 

  #ifndef INJ_CHANNELS
    #define INJ_CHANNELS 4
  #endif
  #ifndef IGN_CHANNELS
    #define IGN_CHANNELS 4
  #endif

extern volatile uint16_t rp2040_dummy_reg;
extern volatile uint16_t picospeed_fuel_registers[8];
extern volatile uint16_t picospeed_ign_registers[8];

  #define FUEL1_COMPARE picospeed_fuel_registers[0]
  #define FUEL2_COMPARE picospeed_fuel_registers[1]
  #define FUEL3_COMPARE picospeed_fuel_registers[2]
  #define FUEL4_COMPARE picospeed_fuel_registers[3]
  #define FUEL5_COMPARE picospeed_fuel_registers[4]
  #define FUEL6_COMPARE picospeed_fuel_registers[5]
  #define FUEL7_COMPARE picospeed_fuel_registers[6]
  #define FUEL8_COMPARE picospeed_fuel_registers[7]

  #define IGN1_COMPARE  picospeed_ign_registers[0]
  #define IGN2_COMPARE  picospeed_ign_registers[1]
  #define IGN3_COMPARE  picospeed_ign_registers[2]
  #define IGN4_COMPARE  picospeed_ign_registers[3]
  #define IGN5_COMPARE  picospeed_ign_registers[4]
  #define IGN6_COMPARE  picospeed_ign_registers[5]
  #define IGN7_COMPARE  picospeed_ign_registers[6]
  #define IGN8_COMPARE  picospeed_ign_registers[7]

  // Semua Counter diarahkan ke rp2040_dummy_reg untuk memuaskan konstruktor struct Schedule
  #define FUEL1_COUNTER rp2040_dummy_reg
  #define FUEL2_COUNTER rp2040_dummy_reg
  #define FUEL3_COUNTER rp2040_dummy_reg
  #define FUEL4_COUNTER rp2040_dummy_reg
  #define FUEL5_COUNTER rp2040_dummy_reg
  #define FUEL6_COUNTER rp2040_dummy_reg
  #define FUEL7_COUNTER rp2040_dummy_reg
  #define FUEL8_COUNTER rp2040_dummy_reg

  #define IGN1_COUNTER  rp2040_dummy_reg
  #define IGN2_COUNTER  rp2040_dummy_reg
  #define IGN3_COUNTER  rp2040_dummy_reg
  #define IGN4_COUNTER  rp2040_dummy_reg
  #define IGN5_COUNTER  rp2040_dummy_reg
  #define IGN6_COUNTER  rp2040_dummy_reg
  #define IGN7_COUNTER  rp2040_dummy_reg
  #define IGN8_COUNTER  rp2040_dummy_reg

  static inline void FUEL1_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL2_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL3_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL4_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL5_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL6_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL7_TIMER_ENABLE(void) { do {} while(0); }
  static inline void FUEL8_TIMER_ENABLE(void) { do {} while(0); }

  static inline void FUEL1_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL2_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL3_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL4_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL5_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL6_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL7_TIMER_DISABLE(void) { do {} while(0); }
  static inline void FUEL8_TIMER_DISABLE(void) { do {} while(0); }

  // --- 2. PELATUK FUNGSIONAL UNTUK MANAGEMEN KOIL PENGAPIAN ---
  static inline void IGN1_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN2_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN3_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN4_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN5_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN6_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN7_TIMER_ENABLE(void) { do {} while(0); }
  static inline void IGN8_TIMER_ENABLE(void) { do {} while(0); }

  static inline void IGN1_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN2_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN3_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN4_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN5_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN6_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN7_TIMER_DISABLE(void) { do {} while(0); }
  static inline void IGN8_TIMER_DISABLE(void) { do {} while(0); }

/*
***********************************************************************************************************
* PICOSPEED DMA INTERCEPTOR DECLARATION
***********************************************************************************************************
*/

  void picospeed_hardware_dma_intercept(uint32_t compare_reg_addr, uint32_t scheduled_val, uint16_t duration_us);

/*
***********************************************************************************************************
* Hardware PWM Direct Macro Injection untuk RP2040 (Boost, VVT, Idle, Fan)
***********************************************************************************************************
*/
class fastInputPin_t;
using boardInputPin_t = fastInputPin_t;
class fastOutputPin_t;
using boardOutputPin_t = fastOutputPin_t;
// Aktifkan Fitur PWM Fan untuk Arsitektur RP2040 secara penuh

#define PWM_FAN_AVAILABLE

void picospeed_set_idle_pwm(void);
void picospeed_disable_idle_pwm(void);
void picospeed_set_boost_pwm(void);
void picospeed_disable_boost_pwm(void);
void picospeed_set_vvt_pwm(void);
void picospeed_disable_vvt_pwm(void);
void picospeed_set_fan_pwm(void);
void picospeed_disable_fan_pwm(void);

#define IDLE_TIMER_ENABLE()   picospeed_set_idle_pwm()
#define IDLE_TIMER_DISABLE()  picospeed_disable_idle_pwm()
#define ENABLE_BOOST_TIMER()  picospeed_set_boost_pwm()
#define DISABLE_BOOST_TIMER() picospeed_disable_boost_pwm()
#define ENABLE_VVT_TIMER()    picospeed_set_vvt_pwm()
#define DISABLE_VVT_TIMER()   picospeed_disable_vvt_pwm()
#define ENABLE_FAN_TIMER()    picospeed_set_fan_pwm()
#define DISABLE_FAN_TIMER()   picospeed_disable_fan_pwm()

// Matikan Sisa Register Pembanding Lama Arsitektur AVR (Bypass Ke Variabel Dummy)
#define BOOST_TIMER_COMPARE   rp2040_dummy_reg
#define BOOST_TIMER_COUNTER   0 
#define VVT_TIMER_COMPARE     rp2040_dummy_reg
#define VVT_TIMER_COUNTER     0 
#define IDLE_COUNTER          0
#define IDLE_COMPARE          rp2040_dummy_reg
#define FAN_TIMER_COUNTER     0
#define FAN_TIMER_COMPARE     rp2040_dummy_reg
/*
***********************************************************************************************************
* CAN / Second serial
*/
#define SECONDARY_SERIAL_T decltype(Serial)
#define RTC_LIB_H <time.h>
constexpr uint16_t BLOCKING_FACTOR = 251;
constexpr uint16_t TABLE_BLOCKING_FACTOR = 256;



/** @brief Analog pin mapping */
constexpr uint8_t ANALOG_PINS[] = { A0, A1, A2, A3 };

/** @brief When the serial buffer is filled to greater than this threshold */
constexpr uint8_t SERIAL_BUFFER_THRESHOLD = 0U;

#endif // END PICO_RP2040