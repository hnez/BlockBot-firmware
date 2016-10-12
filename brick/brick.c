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

char ans='\0';

bool gibberish_tx(uint8_t *bte)
{
  *bte= ans;

  return(true);
}

bool rcv_cb(uint8_t bte)
{
  ans= bte;

  return(true);
}

int main (void)
{
  led_init();
  led_set(1, true);

  uart_init();
  uart.cb_tx= gibberish_tx;
  uart.cb_rx= rcv_cb;

  uart_start_passive();

  sei();

  for(;;) {


    _delay_ms(500);
  }

  return (0);
}
