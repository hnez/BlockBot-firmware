#pragma once

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


inline void buttons_init(void)
{
  // Enable Pullups
  
  BUTTON_PWR_PORT|= _BV(BUTTON_PWR_NUM);
  BUTTON_PLAY_PORT|= _BV(BUTTON_PLAY_NUM);
}
