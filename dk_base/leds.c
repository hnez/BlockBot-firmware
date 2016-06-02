#include <avr/io.h>

#include <vm.h>

#include "leds.h"


uint8_t mem_getled (__attribute__((unused)) struct vm_status_t *vm,
                    __attribute__((unused)) uint8_t addr,
                    uint8_t *val)
{
  *val|= (LED_PORT & 0x01) ? _BV(0) : 0;

  return (MEM_OK);
}

uint8_t mem_setled (__attribute__((unused)) struct vm_status_t *vm,
                    __attribute__((unused)) uint8_t addr,
                    uint8_t val)
{
  LED_PORT= (LED_PORT & ~_BV(LED_NUM))
    | ((val & 0x01) ? _BV(LED_NUM) : 0);

  return (MEM_OK);
}
