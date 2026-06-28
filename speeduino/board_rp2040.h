#pragma once

#include <Arduino.h>


#define USE_PIO_PIN
#ifndef LED_BUILTIN
  #define LED_BUILTIN 25
#endif
#ifndef INJ_CHANNELS
  #define INJ_CHANNELS 4
#endif
#ifndef IGN_CHANNELS
  #define IGN_CHANNELS 4
#endif
/*
***********************************************************************************************************
* General
*/
  #define CUSTOM_BOARD //Disable force to use board 0.4 on Init.cpp
  #define TS_SERIAL_BUFFER_SIZE (256+7+1) //Size of the serial buffer used by new comms protocol. For SD transfers this must be at least 512 + 1 (flag) + 4 (sector)
  #define FPU_MAX_SIZE 0 //Size of the FPU buffer. 0 means no FPU.
  
  #define EEPROM_LIB_H <EEPROM.h>
  class EEPROMClass;
  using EEPROM_t = EEPROMClass;

  constexpr uint16_t BLOCKING_FACTOR = 121;
  constexpr uint16_t TABLE_BLOCKING_FACTOR = 64;
  constexpr uint8_t SERIAL_BUFFER_THRESHOLD = 0U;

/*
***********************************************************************************************************
* Pins Peripheral
*/

  #define BOARD_MAX_IO_PINS  28 //digital pins + analog channels + 1
  #define BOARD_MAX_DIGITAL_PINS 28 //Pretty sure this isn't right
  // Hack AVR unused pins Define
  #ifndef A8
    #define A4  18U
    #define A5  19U
    #define A6  20U
    #define A7  21U
    #define A8  22U
    #define A9  23U
    #define A15 15U
  #endif

  // Forbidden pins like USB
  static inline bool pinIsReserved(uint8_t pin) { 
    return pin==23U; 
  } 

  #define SECONDARY_SERIAL_T SerialUART
  constexpr uint8_t ANALOG_PINS[] = { _ANALOG_PINS_RP2040 };
  class fastInputPin_t;
  using boardInputPin_t = fastInputPin_t;
  class fastOutputPin_t;
  using boardOutputPin_t = fastOutputPin_t;

/*
***********************************************************************************************************
* Dummycounter(fungsi sementara agar lolos build sebelum logic calculation dibuat )
*/
#define ticksToMicros(ticks) (ticks)
#define uS_TO_TIMER_COMPARE(us) (us)
#define COMPARE_TYPE uint32_t
#define COUNTER_TYPE uint32_t
typedef volatile uint32_t compare_t;
//constexpr static inline uint32_t ticksToMicros(uint32_t ticks) { return ticks; }

extern volatile uint32_t dummy_boost_compare;
extern volatile uint32_t dummy_boost_counter;
extern volatile uint32_t dummy_vvt_compare;
extern volatile uint32_t dummy_vvt_counter;
extern volatile uint32_t dummy_timer_compare;
extern volatile uint32_t dummy_timer_counter;
extern volatile uint32_t dummy_ign_compare;
extern volatile uint32_t dummy_ign_counter;
extern volatile uint32_t dummy_inj_compare;
extern volatile uint32_t dummy_inj_counter;
extern volatile uint32_t dummy_idle_compare;
extern volatile uint32_t dummy_idle_counter;

/*
***********************************************************************************************************
* Schedules
*/

  
  #define FUEL1_COUNTER dummy_inj_counter
  #define FUEL2_COUNTER dummy_inj_counter
  #define FUEL3_COUNTER dummy_inj_counter
  #define FUEL4_COUNTER dummy_inj_counter
  //The below are optional, but recommended if there are sufficient timers/compares
  #define FUEL5_COUNTER dummy_inj_counter
  #define FUEL6_COUNTER dummy_inj_counter
  #define FUEL7_COUNTER dummy_inj_counter
  #define FUEL8_COUNTER dummy_inj_counter

  #define IGN1_COUNTER  dummy_ign_counter
  #define IGN2_COUNTER  dummy_ign_counter
  #define IGN3_COUNTER  dummy_ign_counter
  #define IGN4_COUNTER  dummy_ign_counter
  //The below are optional, but recommended if there are sufficient timers/compares
  #define IGN5_COUNTER  dummy_ign_counter
  #define IGN6_COUNTER  dummy_ign_counter
  #define IGN7_COUNTER  dummy_ign_counter
  #define IGN8_COUNTER  dummy_ign_counter

  #define FUEL1_COMPARE dummy_inj_compare
  #define FUEL2_COMPARE dummy_inj_compare
  #define FUEL3_COMPARE dummy_inj_compare
  #define FUEL4_COMPARE dummy_inj_compare
  //The below are optional, but recommended if there are sufficient timers/compares
  #define FUEL5_COMPARE dummy_inj_compare
  #define FUEL6_COMPARE dummy_inj_compare
  #define FUEL7_COMPARE dummy_inj_compare
  #define FUEL8_COMPARE dummy_inj_compare

  #define IGN1_COMPARE  dummy_ign_compare
  #define IGN2_COMPARE  dummy_ign_compare
  #define IGN3_COMPARE  dummy_ign_compare
  #define IGN4_COMPARE  dummy_ign_compare
  //The below are optional, but recommended if there are sufficient timers/compares
  #define IGN5_COMPARE  dummy_ign_compare
  #define IGN6_COMPARE  dummy_ign_compare
  #define IGN7_COMPARE  dummy_ign_compare
  #define IGN8_COMPARE  dummy_ign_compare

  static inline void FUEL1_TIMER_ENABLE(void)  {}
  static inline void FUEL2_TIMER_ENABLE(void)  {}
  static inline void FUEL3_TIMER_ENABLE(void)  {}
  static inline void FUEL4_TIMER_ENABLE(void)  {}
  //The below are optional, but recommended if there are sufficient timers/compares
  static inline void FUEL5_TIMER_ENABLE(void)  {}
  static inline void FUEL6_TIMER_ENABLE(void)  {}
  static inline void FUEL7_TIMER_ENABLE(void)  {}
  static inline void FUEL8_TIMER_ENABLE(void)  {}

  static inline void FUEL1_TIMER_DISABLE(void)  {}
  static inline void FUEL2_TIMER_DISABLE(void)  {}
  static inline void FUEL3_TIMER_DISABLE(void)  {}
  static inline void FUEL4_TIMER_DISABLE(void)  {}
  //The below are optional, but recommended if there are sufficient timers/compares
  static inline void FUEL5_TIMER_DISABLE(void)  {}
  static inline void FUEL6_TIMER_DISABLE(void)  {}
  static inline void FUEL7_TIMER_DISABLE(void)  {}
  static inline void FUEL8_TIMER_DISABLE(void)  {}

    static inline void IGN1_TIMER_ENABLE(void)  {}
    static inline void IGN2_TIMER_ENABLE(void)  {}
    static inline void IGN3_TIMER_ENABLE(void)  {}
    static inline void IGN4_TIMER_ENABLE(void)  {}
  //The below are optional, but recommended if there are sufficient timers/compares
    static inline void IGN5_TIMER_ENABLE(void)  {}
    static inline void IGN6_TIMER_ENABLE(void)  {}
    static inline void IGN7_TIMER_ENABLE(void)  {}
    static inline void IGN8_TIMER_ENABLE(void)  {}

    static inline void IGN1_TIMER_DISABLE(void)  {}
    static inline void IGN2_TIMER_DISABLE(void)  {}
    static inline void IGN3_TIMER_DISABLE(void)  {}
    static inline void IGN4_TIMER_DISABLE(void)  {}
  //The below are optional, but recommended if there are suffici;}ent timers/compares
    static inline void IGN5_TIMER_DISABLE(void)  {}
    static inline void IGN6_TIMER_DISABLE(void)  {}
    static inline void IGN7_TIMER_DISABLE(void)  {}
    static inline void IGN8_TIMER_DISABLE(void)  {}

  /*
***********************************************************************************************************
* Auxiliaries
*/
#define ENABLE_BOOST_TIMER()  
#define DISABLE_BOOST_TIMER() //TIMSK1 &= ~(1 << OCIE1A)
#define ENABLE_VVT_TIMER()    //TIMSK1 |= (1 << OCIE1B)
#define DISABLE_VVT_TIMER()   //TIMSK1 &= ~(1 << OCIE1B)

#define BOOST_TIMER_COMPARE   dummy_boost_compare
#define BOOST_TIMER_COUNTER   dummy_boost_counter
#define VVT_TIMER_COMPARE     dummy_vvt_compare
#define VVT_TIMER_COUNTER     

/*
***********************************************************************************************************
* Idle
*/
#define IDLE_COMPARE  dummy_idle_compare
#define IDLE_COUNTER  dummy_idle_counter

#define IDLE_TIMER_ENABLE() //TIMSK1 |= (1 << OCIE1C)
#define IDLE_TIMER_DISABLE() //TIMSK1 &= ~(1 << OCIE1C)

/*
***********************************************************************************************************
*/
