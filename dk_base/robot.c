#include <avr/io.h>
#include <util/delay.h>

#include <vm.h>

#include "motor.h"
#include "buttons.h"


uint8_t prog[] = {
  0x09, 0x40,  0xe1,  0xb0, 0x30
};

int main (void)
{
  motor_init();
  buttons_init();

  vm_status.prog= prog;
  vm_status.prog_len= sizeof(prog);

  vm_run();

  return (0);
}
