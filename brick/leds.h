#pragma once

#include <stdbool.h>

#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_1_PIN PB4
#define LED_2_PIN PB3

inline void led_init(void)
{
  LED_DDR|= _BV(LED_1_PIN) | _BV(LED_2_PIN);
}

inline void led_set(uint8_t num, bool on)
{
  if (num==1) {
    LED_PORT= (LED_PORT & ~_BV(LED_1_PIN)) | (on ? _BV(LED_1_PIN) : 0);
  }
  else {
    LED_PORT= (LED_PORT & ~_BV(LED_2_PIN)) | (on ? _BV(LED_2_PIN) : 0);
  }
}
