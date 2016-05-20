#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> /* uart_times */
#include <util/delay.h>
#include <stdbool.h>
#include "queue.h" /* buffer */

/* buffer size */
#define count 50

/* For ATtiny85 */
#define TX_DDR  DDRB
#define TX_PORT PORTB
#define TX_NUM  PB4

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_NUM  PB3

const char my_code[10] = {
  //'0','1','2','3','4','5','6','7','8','9',
  "0123456789"
};

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

uint8_t I_time; // The current time of uart_times
uint16_t I_symbol_In; // The currently incoming symbol
uint16_t I_symbol_Out; // The currently outgoing symbol
struct queue_t buffer;
bool buffer_empty;

void interrupts_init(void){
  /* Seite 81 Timer/Counter Interrupt Mask Register */
  TIMSK = _BV(OCIE0A); /* Timer/Counter0 Output Compare Match A Interrupt Enable */
  I_time = 0;
  I_symbol_In = 0;
  I_symbol_Out = 0;
  buffer_empty = false;
}

ISR(TIMER0_COMPA_vect){

  if(I_time==0){

    //make symbol_Out
    I_symbol_Out = _BV(sizeof(uart_times)-1) | ((uint16_t)pop(&buffer) << 1);

    /* clear last symbol_In */
    I_symbol_In = 0;
  }

  /* receiving */
  if(RX_PIN&_BV(RX_NUM)){ //high
    I_symbol_In |= _BV(I_time);
  }

  /* transmitting */
  if (I_symbol_Out & 0x1) {
    TX_PORT|= _BV(TX_NUM);
  } else {
    TX_PORT&=~_BV(TX_NUM);
  }
  /* Cut off last bit */
  I_symbol_Out>>=1;


  /* Byte recorded */
  if(I_time == sizeof(uart_times)-1){
    /* Stop counting */
    TCCR0B= 0;
    /* reset timer */
    TCNT0=0x00;

    /* init next byte */
    I_time = 0;
    OCR0A = pgm_read_byte(&(uart_times[I_time]));

    /* If endbit is high             and            startbit is low -> byte OK */
    if((I_symbol_In & _BV(sizeof(uart_times)-1)) && (~I_symbol_In & _BV(0))){

      /* Cut off start and stop bit and push */
      I_symbol_In>>=1;
      push(&buffer, (char) (I_symbol_In&0xFF));

      /* wait for next byte with timeout */
      wait_for_byte(true);
    } else {
      _delay_us(65);
    }
    if(buffer_size(&buffer)!=0){
      TCCR0B= _BV(CS01) | _BV(CS00) | _BV(WGM01) | _BV(WGM00); //clk/64 prescale, CTC-Mode Page 72
    }
  } else {
    I_time++;
    /* Page 80 Output Compare Register A */
    OCR0A = pgm_read_byte(&(uart_times[I_time])) - pgm_read_byte(&(uart_times[I_time-1]));
  }
}


int main (void){
  baud_init();
  uart_init();
  interrupts_init();
  queue_init(&buffer);

  for(;;){

    /* Wait until buffer is empty */
    while(buffer_size(&buffer)>0);
    _delay_ms(15); //Finish last cycle


    cli();


    for(uint8_t M_i=0;M_i<sizeof(my_code);M_i++){
      push(&buffer, my_code[M_i]);
    }
    buffer_empty = false;
    I_time = 0;

    if(wait_for_byte(false)){

      /* Page 80 Output Compare Register A */
      OCR0A = pgm_read_byte(&(uart_times[0]));

      TCNT0=0x00; //reset timer
      TCCR0B= _BV(CS01) | _BV(CS00) | _BV(WGM01) | _BV(WGM00); //clk/64 prescale, CTC-Mode Page 72

      /* set Global Interrupt Enable */
      sei();
    }

    /* Other code */

  }
}
