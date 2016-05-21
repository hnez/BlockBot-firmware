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
volatile uint8_t I_bit; // The current time of uart_times
volatile uint16_t I_symbol_In; // The currently incoming symbol
volatile uint16_t I_symbol_Out; // The currently outgoing symbol
volatile uint16_t UART_mode;
struct queue_t buffer;

const char my_code[10] = { /* Machinecode of particular block */
  //'0','1','2','3','4','5','6','7','8','9',
  "0123456789"
};

const uint8_t TX_times[] PROGMEM = { //for 1.6MHz atTiny with clk/64
  6,  // Start bit
  20, 32, 46, 60, 72, 84, 100, 110, // Data bits
  124, // Stop bit
};
const uint8_t RX_times[] PROGMEM = { //for 1.6MHz atTiny with clk/64
  13,  // Start bit
  26, 39, 53, 66, 78, 92, 105, 117, // Data bits
  131, // Stop bit
};

void cpuclk_init();
void uart_init();
void interrupts_init();
void setup_buffer();
void next_byte();

ISR(PCINT0_vect){
  /* This Interrupt starts transfering per byte */
  /* if high to low-> new Byte */
  if(I_bit==0 && (~RX_PIN&_BV(RX_NUM))){

    TX_PORT&=~_BV(TX_NUM); // TX startbit
    //I_symbol_In & _BV(0) == 0 therefore no need to record it

    I_bit = 1; //TX first databit
    OCR0A = pgm_read_byte(&(TX_times[I_bit]));
    UART_mode = TX_NUM;

    TCNT0 = pgm_read_byte(&(TX_times[0]))+1; //sync with RX startbit
    TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale

    PORTB|=_BV(PB0); //debug
  }
}

ISR(TIMER0_COMPA_vect){
  if(UART_mode==TX_NUM){

    /* transmitting */
    if (I_symbol_Out & _BV(I_bit)) {
      TX_PORT|= _BV(TX_NUM);
    } else {
      TX_PORT&=~_BV(TX_NUM);
    }
    OCR0A = pgm_read_byte(&(RX_times[I_bit]));
    UART_mode = RX_NUM;

  } else if (UART_mode==RX_NUM){

    /* receiving */
    if(RX_PIN&_BV(RX_NUM)){ //high
      I_symbol_In |= _BV(I_bit);
    }

    I_bit++;
    if(I_bit<sizeof(RX_times)){
      //continue transfer
      OCR0A = pgm_read_byte(&(TX_times[I_bit]));
      UART_mode = TX_NUM;
    } else {
      next_byte();
    }
  }
}

void next_byte(){
  /* If endbit is high             and            startbit is low -> byte OK */
  if((I_symbol_In & _BV(sizeof(RX_times)-1)) && (~I_symbol_In & _BV(0))){


    I_symbol_In>>=1;    /* Cut off start and stop bit and push */
    push(&buffer, (char) (I_symbol_In&0xFF));

  }
  I_symbol_In = 0;
  I_bit = 0;

   if(buffer_size(&buffer)>0){ //force next byte
    I_symbol_Out = _BV(sizeof(TX_times)-1) | ((uint16_t)pop(&buffer) << 1);
    TCCR0B= 0;
    OCR0A = pgm_read_byte(&(TX_times[0]));
    UART_mode = TX_NUM;
    TCNT0 = 0;
    _delay_us(69); //time between bytes
    TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale

   } else { //this happens when receiving has stoped and the buffer ran out
     TCCR0B= 0; //Stop transfer
     setup_buffer(); // prepare bufffer for nexter transfer
     I_symbol_Out = _BV(sizeof(TX_times)-1) | ((uint16_t)pop(&buffer) << 1);
     PORTB&=~_BV(PB0); //debug
   }
}


//ISR(TIMER0_OVF_vect){}


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



void interrupts_init(){ //TODO
  /* Pin Change Interrupt Enable */
  GIMSK|=_BV(PCIE); /* Page 51 */
  PCMSK|=_BV(RX_Interrupt); /* Page 52 */

  setup_buffer();
  I_bit = 0;
  I_symbol_Out = _BV(sizeof(TX_times)-1) | ((uint16_t)pop(&buffer) << 1);
  I_symbol_In = 0;
  UART_mode = TX_NUM;


  TIMSK |= _BV(OCIE0A); /* Timer/Counter0 Output Compare Match A Interrupt Enable */
  // TIMSK |= _BV(TOIE0); /* Timer/Counter0 Overflow Interrupt Enable */
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
  TX_PORT|=_BV(TX_NUM); //high
}
