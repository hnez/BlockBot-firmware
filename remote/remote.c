#ifndef __UNIT_TEST__
 #include <avr/io.h>
 #include <util/delay.h>
#endif

#include "led.h"
#include "power.h"

int main (void)
{
  pwr_self(POWER_ON);

  for (;;) {
    if (pwr_chkovc()) {
      led_error(LED_EOVERCURR);
    }
  }

  return (0);
}
