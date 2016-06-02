#include <avr/io.h>

#include "buttons.h"

uint8_t mem_getbtn (__attribute__((unused)) struct vm_status_t *vm,
                    __attribute__((unused)) uint8_t addr,
                    uint8_t *val)
{
  register uint8_t btn=0;

  if (button_play_pressed()) {
    btn|= _BV(1);
  }

  if (button_pwr_pressed()) {
    btn|= _BV(2);
  }

  *val= btn;

  return (MEM_OK);
}
