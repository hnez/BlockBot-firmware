#include <avr/io.h>
#include <util/delay.h>

#include "motor.h"
#include "buttons.h"

int main (void)
{
  motor_init();
  buttons_init();

  for (;;) {
    
    if (button_play_pressed()) {
      motor_1_set(60);
      motor_2_set(60);

      _delay_ms(200);
    }

    if (button_pwr_pressed()) {
      motor_1_set(-60);
      motor_2_set(-60);

      _delay_ms(200);
    }
  }
}
