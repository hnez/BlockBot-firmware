#pragma once

#include <vm.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define BUTTON_PWR_PORT PORTA
#define BUTTON_PWR_PIN  PINA
#define BUTTON_PWR_NUM  PA2

#define BUTTON_PLAY_PORT PORTA
#define BUTTON_PLAY_PIN  PINA
#define BUTTON_PLAY_NUM  PA3

inline uint8_t button_play_pressed(void)
{
  return !(BUTTON_PLAY_PIN & _BV(BUTTON_PLAY_NUM));
}

inline uint8_t button_pwr_pressed(void)
{
  return !(BUTTON_PWR_PIN & _BV(BUTTON_PWR_NUM));
}

uint8_t mem_getbtn (struct vm_status_t *, uint8_t, uint8_t *);

inline void buttons_init(void)
{
  // Enable Pullups

  BUTTON_PWR_PORT|= _BV(BUTTON_PWR_NUM);
  BUTTON_PLAY_PORT|= _BV(BUTTON_PLAY_NUM);
}

uint8_t mem_getdin (uint8_t);
