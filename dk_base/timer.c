#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <vm.h>

#include "timer.h"
#include "motor.h"

#define PWM_FREQ (F_CPU/MOT_PWM_PRESCALE/(MOT_PWM_TOP+1))
#define TMR_FREQ (50L)
#define STEPSIZE (TMR_FREQ*(1L<<16)/PWM_FREQ)

int32_t timer=0;

void timer_init(void)
{
  TIMSK0|= _BV(TOIE0);
}

uint8_t mem_gettimer (__attribute__((unused)) struct vm_status_t *vm,
                      __attribute__((unused)) uint8_t addr,
                      uint8_t *val)
{
  *val= ((timer>>16) & 0x00ff);

  return (MEM_OK);
}

uint8_t mem_settimer (__attribute__((unused)) struct vm_status_t *vm,
                      __attribute__((unused)) uint8_t addr,
                      uint8_t val)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    timer= ((uint32_t)val+1)<<16;
  }

  return (MEM_OK);
}

ISR(TIMER1_OVF_vect)
{
  timer-=STEPSIZE;

  if (timer < 0) timer=0;
}
