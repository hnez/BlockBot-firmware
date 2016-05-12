#include <avr/io.h>

#include "buttons.h"

uint8_t mem_getdin (__attribute__((unused)) uint8_t addr)
{
  register uint8_t btn=0;

  if (button_play_pressed()) {
    btn|= _BV(1);
  }

  if (button_pwr_pressed()) {
    btn|= _BV(2);
  }

  return (btn);
}
