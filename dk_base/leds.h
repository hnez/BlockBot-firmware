#pragma once

#include <vm.h>

#define LED_DDR  DDRD
#define LED_PORT PORTD
#define LED_NUM  PD7

#ifndef NULL
#define NULL ((void*)0)
#endif

uint8_t mem_getled (struct vm_status_t *, uint8_t, uint8_t *);
uint8_t mem_setled (struct vm_status_t *, uint8_t, uint8_t);

inline void leds_init(void)
{
  LED_DDR |= _BV(LED_NUM);
}
