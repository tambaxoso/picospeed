#if defined(PICO_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include "pico/multicore.h"
#include "src/rp2040_pio/pio_driver.h"
#include "globals.h"

void setup1(void)
{
 
}

void __not_in_flash_func(loop1(void))
{
  
}

#endif