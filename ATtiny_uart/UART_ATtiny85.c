
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdbool.h> //könnte man um bytes zu sparen auch alles mit ints machen

/* atTiny85 */
#define TX_DDR  DDRB
#define TX_PORT PORTB
#define TX_NUM  PB4

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_NUM  PB3

const uint8_t uart_times[] PROGMEM = { //for 1.6MHz atTiny with clk/64
  6,  // Start bit
  20, 32, 46, 60, 72, 84, 100, 110, // Data bits
  124, // Stop bit
};


void uart_putc(char c){
  //Reset and start timer
  TCNT0=0x00;
  TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale


  // merge in start (LOW) and stop (HIGH) bit
  uint16_t symbol=  _BV(9) | ((uint16_t)c << 1); // int16 weil 8 bits
                   //HIGH9    c als int  LOW0
  for (uint8_t i=0;i<sizeof(uart_times);i++) {
    register uint8_t curtime;
    //for heavy use, Sollte der compiler selber machen

    curtime= pgm_read_byte(&(uart_times[i]));
    //Read a byte from the program space with a 16-bit (near) address.

    // wait for timer
    while (TCNT0<curtime);

    if (symbol & 0x01) {
      TX_PORT|= _BV(TX_NUM);
    }
    else {
      TX_PORT&=~_BV(TX_NUM);
    }
    symbol>>=1; // Schneidet das Ende ab
  }

  // stop timer
  TCCR0B= 0;
}

void uart_puts(char *str){
  while (*str) {
    uart_putc(*str++);
    _delay_us(65);
  }
}

bool wait_for_byte(bool timeout){ // no timeout false

  if(~RX_PIN&_BV(RX_NUM)){ //if low
    return false; //no connection
  }

  if(!timeout){ //no timeout
    while(RX_PIN&_BV(RX_NUM)); // while high
    return true;

  } else { //timeout

    TCNT0=0x00; //set timer
    TCCR0B= _BV(CS01) || _BV(CS00); //clk/256

    while(0xFD>TCNT0){
      if(~RX_PIN&_BV(RX_NUM)){ //if low
        TCCR0B= 0;
        return true;
      }
    }
    TCCR0B= 0;
    return false;
  }
}

char uart_getc(void){

  uint16_t symbol = 0;
  TCNT0=0x00; //set timer
  TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale
  register uint8_t curtime;

  for(uint8_t i = 0;i<sizeof(uart_times);i++){

    curtime= pgm_read_byte(&(uart_times[i]));
    while(curtime>TCNT0);

    if(RX_PIN&_BV(RX_NUM)){ //high
      symbol|=_BV(i);
    }
  }

  //cut of start and stop bit
  symbol>>=1;
  return (char) (symbol&0xFF);

  //return byte;
}

void uart_get(char * container){
  bool broadcasting = true;
  uint8_t i;
  for(i = 0;broadcasting;i++){
    container[i] = uart_getc();
    if(!wait_for_byte(true)){ //max, about 70us
      broadcasting = false;
    }
  }
}

void uart_init(void){
  TX_DDR|=_BV(TX_NUM); //put
  RX_DDR&=~_BV(RX_NUM); //get
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

int main (void){
  baud_init();
  uart_init();

  char msg[20];
  for (;;) {
    if(wait_for_byte(false)){
      uart_get(&msg[0]);
      uart_puts(msg);
      for(uint8_t xx=0;xx<sizeof(msg);xx++)
        msg[xx]=0; // I dont want to import string.h for memset
    }
  }
}
