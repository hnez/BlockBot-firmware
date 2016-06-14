#ifndef __UNIT_TEST__
 #include <avr/io.h>
 #include <util/delay.h>
#endif

#include "power.h"

#include "led.h"

void pwr_self(uint8_t is_on)
{
  if (is_on) {
    DDRB |= _BV(PB1);
    PORTB |= _BV(PB1);
  }
  else {
    PORTB &= ~_BV(PB1);
    for (;;);
  }
}

void pwr_bricks (uint8_t is_on)
{
  if (is_on) {
    DDRB |= _BV(PB0);
    PORTB |= _BV(PB0);
  }
  else {
    PORTB &= ~_BV(PB0);
  }
}

/* Check for an overcurrent condition.
 * Power of bricks in case of overcurrent */
uint8_t pwr_chkovc (void)
{
  if (PINC & _BV(PC0)) {

    pwr_bricks(POWER_OFF);

    led_error(LED_EOVERCURR);
    return (1);
  }

  return (0);
}
