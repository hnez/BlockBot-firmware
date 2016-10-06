#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <avr/eeprom.h>

  #include <util/delay.h>

  #include "leds.h"
#endif



int main (void)
{
  led_init();

  for(;;) {
    led_set(1, true);
    led_set(2, false);

    _delay_ms(1000);

    led_set(1, false);
    led_set(2, true);

    _delay_ms(1000);
  }

  return (0);
}
