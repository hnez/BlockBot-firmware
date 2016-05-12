
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

const uint8_t uart_times[] PROGMEM = { //for 1.6MHz atTiny
  2,  // Start bit
  5, 8, 12, 15, 18, 21, 25, 28, // Data bits
  31, // Stop bit
};


void uart_putc(char c){
  //Reset and start timer
  TCNT0=0x00;
  //The Timer/Counter Register gives direct access, both for read and write operations, to the Timer/Counter unit 8- bit counter.
  TCCR0B= _BV(CS02);
  //The clock source is selected by the Clock Select logic which is controlled by the Clock Select (CS02:0)
  //bits located in the Timer/Counter Control Register (TCCR0B).
  //The Timer/Counter can be clocked directly by the system clock (by setting the CSn2:0 = 1).

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

  //if(!(RX_PIN&_BV(RX_NUM))){ //if low
  //  return false; //no connection
  //}

  if(!timeout){ //no timeout
    while(RX_PIN&_BV(RX_NUM)); // while high
    return true;

  } else { //timeout

    TCNT0=0x00; //set timer
    TCCR0B= _BV(CS02); //set timersource

    while(0xFA>TCNT0){
      if(!(RX_PIN&_BV(RX_NUM))){
        return true;
      }
    }
    return false;
  }
}

char uart_getc(void){

  char byte = 0;
  uint16_t symbol = 0;
  TCNT0=0x00; //set timer
  TCCR0B= _BV(CS02); //set timersource
  register uint8_t curtime;

  for(uint8_t i = 0;i<sizeof(uart_times);i++){

    curtime= pgm_read_byte(&(uart_times[i]));
    while(curtime>TCNT0);

    if(RX_PIN&_BV(RX_NUM)){ //high
      symbol|=_BV(i);
    }
  }

  for(uint8_t j = 1;j<sizeof(uart_times)-1;j++){
    if(symbol&_BV(j)){
      byte|=_BV(j-1);
    }
  }

  return byte;
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
    }
  }
}
