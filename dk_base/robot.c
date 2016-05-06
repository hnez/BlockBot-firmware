#include <avr/io.h>
#include <util/delay.h>

#include <vm.h>

#include "motor.h"
#include "buttons.h"
#include "suart.h"

#define NULL ((void *)0)

uint8_t mem_getmot (__attribute__((unused)) uint8_t addr)
{
  return (0);
}

uint8_t mem_setmot (uint8_t addr, uint8_t val)
{
  if (addr==0) {
    uart_puts("Motor 1");
    motor_1_set(val);
  }
  else {
    uart_puts("Motor 2");
    motor_2_set(val);
  }

  return (MEM_OK);
}

PROGMEM const struct mem_slot mem_map[8]= {
  {mem_getmot,   mem_setmot},
  {mem_getmot,   mem_setmot},
  {NULL,   NULL},
  {NULL,   NULL},
  {NULL,   NULL},
  {NULL,   NULL},
  {NULL,   NULL},
  {NULL,   NULL}
};

uint8_t prog[] = {
  0x09, 0x1e, 0xe1, 0xe5
};

int main (void)
{
  uart_init();
  motor_init();
  buttons_init();

  uart_puts("Motoren init\r\n");
  motor_1_set(0);
  motor_2_set(0);

  vm_status.pc=0;
  vm_status.prog= prog;
  vm_status.prog_len= sizeof(prog)+1;

  uart_puts("Run\r\n");
  for(;;) {
    vm_step();  
  
    _delay_ms(1000);
  }

  return (0);
}
