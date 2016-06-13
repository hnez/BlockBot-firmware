#ifndef __UNIT_TEST__
 #include <avr/io.h>
 #include <util/delay.h>
#endif

#include "power.h"
#include "led.h"

/* Blink out an error code in a loop
 * and send the uC to death afterwards*/
void led_error (uint8_t errcode)
{
  for (uint8_t times=0; times<8; times++) {
    for (uint8_t i=0; i<errcode; i++) {
      PORTC &= ~_BV(PC1);
      _delay_ms(100);

      PORTC |= _BV(PC1);
      _delay_ms(100);
    }

    _delay_ms(500);
  }

  pwr_self(POWER_OFF);
}

void led_init (void)
{
  DDRC |= _BV(PC1);
  PORTC |= _BV(PC1);
}
