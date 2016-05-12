#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <vm.h>

#include "timer.h"

#define PWM_FREQ (F_CPU/8/256)
#define TMR_FREQ (60L)
#define STEPSIZE (TMR_FREQ*(1L<<16)/PWM_FREQ)

int32_t timer;

uint8_t mem_gettimer (__attribute__((unused)) uint8_t addr)
{
  return ((timer>>16) & 0x00ff);
}

uint8_t mem_settimer (__attribute__((unused)) uint8_t addr, uint8_t val)
{
  register uint8_t old= SREG;
  cli();
  timer= ((uint32_t)val+1)<<16;
  SREG= old;

  return (MEM_OK);
}

ISR(TIM0_OVF_vect)
{
  timer-=STEPSIZE;

  if (timer < 0) timer=0;
}
