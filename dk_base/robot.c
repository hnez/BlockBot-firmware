#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <vm.h>

#include "motor.h"
#include "buttons.h"
#include "timer.h"
#include "leds.h"

PROGMEM const struct mem_slot mem_map[8]= {
  {mem_getmot,   mem_setmot},
  {mem_getmot,   mem_setmot},
  {NULL, NULL},
  {NULL, NULL},
  {mem_gettimer, mem_settimer},
  {NULL, NULL},
  {NULL, NULL},
  {NULL, NULL}
};

int main (void)
{
  static struct vm_status_t vm;

  motor_init();
  buttons_init();
  leds_init();
  timer_init();

  sei();

  vm.pc=0;
  vm.prog= NULL;
  vm.prog_len= 0;

  vm_run(&vm);

  for (;;);

  return (0);
}
