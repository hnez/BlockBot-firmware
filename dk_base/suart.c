#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


#define UARTTX_DDR  DDRB
#define UARTTX_PORT PORTB
#define UARTTX_NUM  PB0

const uint8_t uart_times[] PROGMEM = {
  3,  // Start bit
  10, 16, 23, 30, 36, 42, 50, 55, // Data bits
  62, // Stop bit
};

void uart_putc(char c)
{
  // Reset and start timer
  TCNT1=0x00;
  TCCR1B= _BV(CS11) | _BV(CS10);

  // merge in start (LOW) and stop (HIGH) bit
  uint16_t symbol=  _BV(9) | ((uint16_t)c << 1);

  for (uint8_t i=0;i<sizeof(uart_times);i++) {
    register uint8_t curtime;
    curtime= pgm_read_byte(&(uart_times[i]));

    // wait for timer
    while (TCNT1<curtime);

    if (symbol & 0x01) {
      UARTTX_PORT|= _BV(UARTTX_NUM);
    }
    else {
      UARTTX_PORT&=~_BV(UARTTX_NUM);  
    }
    symbol>>=1;
  }

  // stop timer
  TCCR1B= 0;
}

void uart_puts(char *str)
{
  while (*str) {
    uart_putc(*str++);
    _delay_ms(1);
  }
}

void uart_init(void)
{
  UARTTX_DDR|=_BV(UARTTX_NUM);
}
