#include <avr/io.h>
#include <util/delay.h>

#include "motor.h"


int main (void)
{
  motor_init();


  for (;;) {
    motor_1_set(255);
    motor_2_set(255);
    _delay_ms(2000);
  }
}
