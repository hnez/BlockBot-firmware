#ifndef __UNIT_TEST__
 #include <avr/io.h>
 #include <util/delay.h>
#endif

#include "led.h"
#include "power.h"

int main (void)
{
  pwr_self(POWER_ON);
  led_init();

  pwr_bricks(POWER_ON);

  // TODO

  pwr_self(POWER_OFF);

  return (0);
}
