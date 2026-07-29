#include "board_definition.h"

#if defined(ARDUINO_ARCH_RP2040)

#include <Arduino.h>
#include <EEPROM.h>
#include "board_eeprom_adapter.hpp"
#include "timers.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <hardware/watchdog.h> 
#include <pico/bootrom.h>
#include "globals.h"  
#include "src/rp2040_pio/picospeed_dma.h"
#include "config_pages.h"
//#include "external_adc.h"

// Register virtual 32-bit tunggal untuk memuaskan konstruktor Scheduler di Core 0
volatile uint16_t rp2040_dummy_reg = 0;

static repeating_timer_t main_1ms_timer;

// Callback Interupsi Hardware Timer 1ms
bool __not_in_flash_func(rp2040_1ms_callback)(repeating_timer_t */*rt*/) {
    oneMSInterval(); // Memanggil interupsi interval 1ms internal Speeduino
    return true; 
}

/**
 * @brief INISIALISASI UTAMA PLATFORM BOARD RP2040
 * Fungsi ini dipanggil sekali oleh speeduino.ino saat pertama kali ECU mendapatkan tegangan.
 */
void initBoard(uint32_t baudRate) {

    picospeed_global_dma_setup();
    Serial.begin(baudRate);
    Serial1.begin(baudRate);
    
    EEPROM.begin(4096);
    add_repeating_timer_us(-1000, rp2040_1ms_callback, NULL, &main_1ms_timer);
    #if defined(USE_EXT_ADC)
    ext_adc_setup();
    #endif
}

uint16_t freeRam() {
    uint32_t free_memory = rp2040.getFreeHeap();
    return (free_memory > UINT16_MAX) ? UINT16_MAX : (uint16_t)free_memory; 
}
void doSystemReset() { 
    rp2040.reboot();
}
void jumpToBootloader() {
    rp2040.rebootToBootloader();
}
uint8_t getSystemTemp() { 
    return (uint8_t)analogReadTemp(); 
}

void boardInitRTC(void) { }

void boardInitPins(uint8_t boardID, pinNumbers_t &pins) {
}

static uint16_t getEepromWriteBlockSize(const statuses &) {
  return 64U;
}

storage_api_t getBoardStorageApi(void) {
  return getEEPROMStorageApi(getEepromWriteBlockSize);
}

/** @brief Get the PWM timer resolution in uS */
uint8_t getPwmTimerResolution(void)
{
  return 1;
}

/*
===========================================================================================================
* 1. IDLE AIR CONTROL (IAC) VALVE MANAGEMENT
===========================================================================================================
*/

void picospeed_set_idle_pwm(void)
{
    // PERBAIKAN: Menambahkan tanda kurung yang hilang di kondisi if dan menyamakan variabel pin
    if (pinNumbers.pinIdle1 < 30) {
        gpio_set_function(pinNumbers.pinIdle1, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinIdle1), true);
    }
    
    // Jika menggunakan mode Katup 3-Wire (Dual Channel), aktifkan juga pin kedua
    if (configPage6.iacChannels == 1 && pinNumbers.pinIdle2 < 30) {
        gpio_set_function(pinNumbers.pinIdle2, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinIdle2), true);
    }
}

void picospeed_disable_idle_pwm(void)
{

    if (pinNumbers.pinIdle1 < 30) { 
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinIdle1), false); 
        gpio_set_function(pinNumbers.pinIdle1, GPIO_FUNC_SIO); 
    }

    if (pinNumbers.pinIdle2 < 30) { 
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinIdle2), false); 
        gpio_set_function(pinNumbers.pinIdle2, GPIO_FUNC_SIO); 
    }
}

/*
===========================================================================================================
* 2. ELECTRONIC BOOST CONTROLLER MANAGEMENT
===========================================================================================================
*/
void picospeed_set_boost_pwm(void)
{
    if (pinNumbers.pinBoost < 30) {
        gpio_set_function(pinNumbers.pinBoost, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinBoost), true);
    }
}

void picospeed_disable_boost_pwm(void)
{
    if (pinNumbers.pinBoost < 30) { 
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinBoost), false); 
        gpio_set_function(pinNumbers.pinBoost, GPIO_FUNC_SIO); 
    }
}

/*
===========================================================================================================
* 3. VARIABLE VALVE TIMING (VVT) SOLENOID MANAGEMENT
===========================================================================================================
*/
void picospeed_set_vvt_pwm(void)
{
    // PERBAIKAN: Mengubah pinVVT_1 menjadi pinNumbers.pinVVT_1
    if (pinNumbers.pinVVT_1 < 30) {
        gpio_set_function(pinNumbers.pinVVT_1, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinVVT_1), true);
    }
    
    // Aktifkan bank VVT kedua jika mesin mengadopsi dual cam VVT (ex: Dual VVTi / VANOS)
    // PERBAIKAN: Mengubah pinVVT_2 menjadi pinNumbers.pinVVT_2
    if (configPage10.vvt2Enabled == 1 && pinNumbers.pinVVT_2 < 30) {
        gpio_set_function(pinNumbers.pinVVT_2, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinVVT_2), true);
    }
}

void picospeed_disable_vvt_pwm(void)
{
    // PERBAIKAN: Mengubah pinVVT_1 menjadi pinNumbers.pinVVT_1
    if (pinNumbers.pinVVT_1 < 30) { 
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinVVT_1), false); 
        gpio_set_function(pinNumbers.pinVVT_1, GPIO_FUNC_SIO); 
    }
    // PERBAIKAN: Mengubah pinVVT_2 menjadi pinNumbers.pinVVT_2
    if (pinNumbers.pinVVT_2 < 30) { 
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinVVT_2), false); 
        gpio_set_function(pinNumbers.pinVVT_2, GPIO_FUNC_SIO); 
    }
}
/*
===========================================================================================================
* 4. RADIATOR ELECTRIC FAN PWM MANAGEMENT
===========================================================================================================
*/
void picospeed_set_fan_pwm(void)
{
    if (configPage2.fanEnable == 2 && pinNumbers.pinFan < 30) {
        gpio_set_function(pinNumbers.pinFan, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinFan), true);
    }
}

void picospeed_disable_fan_pwm(void)
{
    if (pinNumbers.pinFan < 30) { 
        pwm_set_enabled(pwm_gpio_to_slice_num(pinNumbers.pinFan), false); 
        gpio_set_function(pinNumbers.pinFan, GPIO_FUNC_SIO); 
    }
}

#endif // END ARDUINO_ARCH_RP2040