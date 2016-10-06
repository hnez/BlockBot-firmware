#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <avr/eeprom.h>

  #include <util/delay.h>

  #include "leds.h"
  #include "uart.h"
#endif

char *gibberish = "Lol was los bei dir Junge?";

bool gibberish_tx(uint8_t *bte)
{
  static size_t pos=0;
  static bool led_stat=false;

  led_set(2, led_stat);
  led_stat= !led_stat;

  if(gibberish[pos]) {
    *bte=gibberish[pos];

    pos++;

    return(true);
  }
  else {
    pos= 0;

    return(false);
  }
}

int main (void)
{
  led_init();
  led_set(1, true);

  uart_init();
  uart.cb_tx= gibberish_tx;

  sei();

  for(;;) {
    uart_start_active();

    _delay_ms(500);
  }

  return (0);
}
