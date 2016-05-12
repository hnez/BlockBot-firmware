//Hauptsächlich Leonards Code mit Kommentaren von Lenard fur Lenard
/* Useful for debugging */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

/* atTiny85 */
#define UARTTX_DDR  DDRB
#define UARTTX_PORT PORTB
#define UARTTX_NUM  PB4

const uint8_t uart_times[] PROGMEM = { //for 1.6MHz atTiny
  2,  // Start bit
  5, 8, 12, 15, 18, 21, 25, 28, // Data bits
  31, // Stop bit
};

void uart_putc(char c)
{
  //Reset and start timer
  TCNT0=0x00;
  //The Timer/Counter Register gives direct access, both for read and write operations, to the Timer/Counter unit 8- bit counter.
  TCCR0B= _BV(CS02);
  //The clock source is selected by the Clock Select logic which is controlled by the Clock Select (CS02:0)
  //bits located in the Timer/Counter Control Register (TCCR0B).
  //The Timer/Counter can be clocked directly by the system clock (by setting the CSn2:0 = 1).

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
