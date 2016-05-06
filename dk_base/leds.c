#include <avr/io.h>

#include "vm.h"

#define LED_DDR  DDRA
#define LED_PORT PORTA
#define LED_NUM  PA6

uint8_t mem_getled (__attribute__((unused)) uint8_t addr)
{
  register uint8_t leds=0;

  if (LED_PORT & 0x01) {
    leds|= _BV(0);
  }

  return (leds);
}

uint8_t mem_setled (__attribute__((unused)) uint8_t addr, uint8_t val)
{
  if (val & 0x01) {
    LED_PORT |=  _BV(LED_NUM);
  }
  else {
    LED_PORT &= ~_BV(LED_NUM);
  }

  return (MEM_OK);
}

void leds_init(void)
{
  LED_DDR |= _BV(LED_NUM);
}
