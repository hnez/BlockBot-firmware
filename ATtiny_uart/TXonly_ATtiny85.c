//Hauptsächlich Leonards Code mit Kommentaren von Lenard fur Lenard
/* Useful for debugging */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

/* atTiny85 */
#define UARTTX_DDR  DDRB
#define UARTTX_PORT PORTB
#define UARTTX_NUM  PB4


const uint8_t uart_times[] PROGMEM = { //for 1.6MHz atTiny with clk/64
  6,  // Start bit
  20, 32, 46, 60, 72, 84, 100, 110, // Data bits
  124, // Stop bit
};

void uart_putc(char c)
{
  //Reset and start timer
  TCNT0=0x00;
  TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale

  // merge in start (LOW) and stop (HIGH) bit
  uint16_t symbol=  _BV(9) | ((uint16_t)c << 1); // c wegen den shift int16
                   //HIGH9    c als int  LOW0
  for (uint8_t i=0;i<sizeof(uart_times);i++) {
    register uint8_t curtime;
    //for heavy use, Sollte der compiler selber machen

    curtime= pgm_read_byte(&(uart_times[i]));
    //macht byte weil TCNT0 in byte ist

    // wait for timer
    while (TCNT0<curtime);

    if (symbol & 0x01) {
      UARTTX_PORT|= _BV(UARTTX_NUM);
    }
    else {
      UARTTX_PORT&=~_BV(UARTTX_NUM);
    }
    symbol>>=1; // Schneidet das Ende ab
  }

  // stop timer
  TCCR0B= 0;
}

void uart_puts(char *str)
{
  while (*str) {
    uart_putc(*str++);
    _delay_us(65);
  }
}

void uart_init(void)
{
  UARTTX_DDR|=_BV(UARTTX_NUM);
}

void baud_init(void){
  //Page 32
  //Clock Prescale Register CLKPR

  // Clock Prescaler Change Enable
  //Everything else to zero
  CLKPR = _BV(CLKPCE);

  //Clock Division Factor 1: Setting Register 0-4 to zero
  // Clock Prescaler Change Disable
  CLKPR &= ~( _BV(CLKPS3) | _BV(CLKPS2) | _BV(CLKPS1) | _BV(CLKPS0) | _BV(CLKPCE) );
}

int main (void)
{
  baud_init();
  uart_init();


  for (;;) {
    uart_puts("QRSTUVWXYZ");
    _delay_ms(1000);

  }
}
