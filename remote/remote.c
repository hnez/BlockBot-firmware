#ifndef __UNIT_TEST__
 #include <avr/io.h>
 #include <util/delay.h>
#endif

#include "led.h"
#include "power.h"

int main (void)
{
  pwr_self(POWER_ON);

  led_error(5);

  for (;;);

  return (0);
}
