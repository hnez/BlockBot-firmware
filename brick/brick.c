#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <avr/eeprom.h>
  #include <avr/sleep.h>
  #include <avr/wdt.h>

  #include <util/delay.h>

  #include "leds.h"
  #include "uart.h"
  #include "brick_binary.h"
#endif

int tx_pos= 0;

bool binary_transmit(uint8_t *out_b)
{
  if (tx_pos < brick_binary_size) {
    *out_b= pgm_read_byte(&brick_binary[tx_pos]);
    tx_pos++;

    return(true);
  }
  else {
    return(false);
  }
}

void main_master(void)
{
  tx_pos= 0;
  uart.cb_tx= binary_transmit;

  uart_start_active();
  sei();

  for(;;) {
    sleep_mode();

    led_toggle(2);
  }
}

void main_slave(void)
{
  led_set(1, true);

}

void startup_notify(void)
{
  led_set(1, true);
  _delay_ms(100);

  led_set(1, false);
  _delay_ms(200);

  led_set(1, true);
  _delay_ms(100);

  led_set(1, false);
}

inline void bootloader_cleanup(void)
{
  cli();
  PORTB= 0;
  MCUSR= 0;
  WDTCR = (1<<WDCE)|(1<<WDE);
  WDTCR = 0;
}

int main (void)
{
  bootloader_cleanup();

  led_init();

  led_toggle(2);
  
  for(;;) {
    _delay_ms(100);

    led_toggle(1);
    led_toggle(2);
  }

  //uart_init();

  /* Waste some time blinking the LED
   * to let the signal lines settle */
  /*
  startup_notify();

  set_sleep_mode(SLEEP_MODE_IDLE);

  if(uart_carrier_detect()) {
    main_slave();
  }
  else {
    main_master();
  }
  */

  return (0);
}
