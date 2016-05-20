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
#define RX_Interrupt PCINT3

/* For interrupts */
volatile uint8_t I_time; // The current time of uart_times
volatile uint16_t I_symbol_In; // The currently incoming symbol
volatile uint16_t I_symbol_Out; // The currently outgoing symbol
struct queue_t buffer;

const char my_code[10] = { /* Machinecode of particular block */
  //'0','1','2','3','4','5','6','7','8','9',
  "0123456789"
};

const uint8_t uart_times[] PROGMEM = { //for 1.6MHz atTiny with clk/64
  6,  // Start bit
  20, 32, 46, 60, 72, 84, 100, 110, // Data bits
  124, // Stop bit
};

void cpuclk_init();
void uart_init();
void interrupts_init();
void setup_new_byte();
void setup_buffer();


ISR(TIMER0_COMPA_vect){

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

  I_time++;
  OCR0A = pgm_read_byte(&(uart_times[I_time]));
}

ISR(PCINT0_vect){
  /* This Interrupt starts and stops transfering per byte */
  if(I_time==0){

    /* if high to low-> new Byte */
    if(~RX_PIN&_BV(RX_NUM)){
      /* setup_new_byte already done */
      TCNT0 = 0;
      TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale
      PORTB|=_BV(PB0); //debug
    }

  /* stopbit_time - startbit_time - delay - safety_time = 107*/
  } else if(TCNT0>107 && RX_PIN&_BV(RX_NUM)) {
    while(TCNT0<125); //wait until last bit received

    /* If endbit is high             and            startbit is low -> byte OK */
    if((I_symbol_In & _BV(sizeof(uart_times)-1)) && (~I_symbol_In & _BV(0))){

      /* Cut off start and stop bit and push */
      I_symbol_In>>=1;
      push(&buffer, (char) (I_symbol_In&0xFF));
    }

    setup_new_byte();
  }
}


/* If PCINT0 wasnt triggered */
ISR(TIMER0_OVF_vect){
  if(buffer_size(&buffer)==0){ //this happens when receiving has stoped and the buffer ran out
    TCCR0B= 0; //Stop transfer
    TCNT0 = 0;
    setup_buffer(); // prepare bufffer for nexter transfer
    setup_new_byte();
  } else { //This happens when receiving stops
    /* Next Byte */
    setup_new_byte();
    TCNT0 = 0;
    TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale
    PORTB|=_BV(PB0); //debug
  }
}


int main (void){
  cpuclk_init();
  uart_init();
  interrupts_init();
  sei();
  for(;;){

  }
}





void setup_buffer(){
  for(uint8_t M_i=0;M_i<sizeof(my_code);M_i++){
    push(&buffer, my_code[M_i]);
  }
}

void setup_new_byte(){
  I_time = 0;
  OCR0A = pgm_read_byte(&(uart_times[0]));
  I_symbol_Out = _BV(sizeof(uart_times)-1) | ((uint16_t)pop(&buffer) << 1);
  I_symbol_In = 0;
}

void interrupts_init(){ //TODO
  /* Pin Change Interrupt Enable */
  GIMSK|=_BV(PCIE); /* Page 51 */
  PCMSK|=_BV(RX_Interrupt); /* Page 52 */

  setup_buffer();
  setup_new_byte();
  TIMSK |= _BV(OCIE0A); /* Timer/Counter0 Output Compare Match A Interrupt Enable */
  TIMSK |= _BV(TOIE0); /* Timer/Counter0 Overflow Interrupt Enable */
}


void cpuclk_init(){
  //Page 32
  //Clock Prescale Register CLKPR

  // Clock Prescaler Change Enable
  //Everything else to zero
  CLKPR = _BV(CLKPCE);

  //Clock Division Factor 1: Setting Register 0-4 to zero
  // Clock Prescaler Change Disable
  CLKPR &= ~( _BV(CLKPS3) | _BV(CLKPS2) | _BV(CLKPS1) | _BV(CLKPS0) | _BV(CLKPCE) );
}

void uart_init(){
  TX_DDR|=_BV(TX_NUM); //put
  RX_DDR&=~_BV(RX_NUM); //get
}
