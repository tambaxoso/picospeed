#include "board_definition.h"

#ifdef ARDUINO_ARCH_RP2040
#include <malloc.h>
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#ifdef USE_SPI_EEPROM
  #include "src/SPIAsEEPROM/SPIAsEEPROM.h"
#else
  #include <EEPROM.h>
#endif

volatile uint32_t dummy_boost_compare = 0;
volatile uint32_t dummy_boost_counter = 0;
volatile uint32_t dummy_vvt_compare = 0;
volatile uint32_t dummy_vvt_counter = 0;
volatile uint32_t dummy_idle_compare = 0;
volatile uint32_t dummy_idle_counter = 0;
volatile uint32_t dummy_ign_compare = 0;
volatile uint32_t dummy_ign_counter = 0;
volatile uint32_t dummy_inj_compare = 0;
volatile uint32_t dummy_inj_counter = 0;

uint8_t getSystemTemp(void) {
    #if defined(ARDUINO_ARCH_RP2040)
        return (uint8_t)analogReadTemp(); // Uses RP2040 internal temperature sensor
    #else
        return 0;
    #endif
}

void initEEPROM() 
{
  // Allocates a 4096-byte layout chunk inside RAM to mirror the flash sector
  EEPROM.begin(4096); 
}
uint16_t freeRam() 
{ 
  uint32_t free_heap = rp2040.getFreeHeap();
    // Cap at 4095 as requested by your system architecture
    if (free_heap > 4096) { return 4096; }
    return (uint16_t)free_heap;
}
static byte rp2040_eeprom_read(uint16_t address) { return EEPROM.read(address); }
static void rp2040_eeprom_write(uint16_t address, byte val) { EEPROM.write(address, val); }
static uint16_t rp2040_eeprom_length(void) { return EEPROM.length(); }
static uint16_t rp2040_getMaxWriteBlockSize(const statuses &current) { return 1; }
static storage_api_t rp2040_storage = {
    .read = rp2040_eeprom_read,
    .write = rp2040_eeprom_write,
    .length = rp2040_eeprom_length,
    .getMaxWriteBlockSize = rp2040_getMaxWriteBlockSize
};
storage_api_t getBoardStorageApi(void){ return rp2040_storage; }
void boardInitPins(uint8_t boardID) {}
void doSystemReset() { return rp2040.reboot(); }
void jumpToBootloader() { return rp2040.rebootToBootloader(); }
void initBoard(uint32_t baudRate)
{
  Serial.begin(115200);
  initEEPROM();
}


#endif