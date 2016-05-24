#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> /* uart_times */
#include <util/delay.h>
#include <stdbool.h>
#include <rdbuf.h> /* buffer */

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

#define RX_MNEM_LEN 10
#define RX_CKSUM 11
#define RX_PAYLOAD 12

/* For interrupts */
volatile uint8_t I_bit; // The current time of uart_times
volatile uint16_t I_symbol_In; // The currently incoming symbol
volatile uint16_t I_symbol_Out; // The currently outgoing symbol
volatile uint8_t UART_mode;
volatile uint8_t RX_TX;
struct queue_t buffer;
char packet_head_In[6]; /*Mnemonic,Length,optional Checksum*/
char packet_head_Out[6]; /*Mnemonic,Length,optional Checksum*/
volatile uint8_t packet_index_In;
volatile uint16_t packet_length; //longer than 255? TOTHINKABOUT

const char my_code[10] = { /* Machinecode of particular block */
  //'0','1','2','3','4','5','6','7','8','9',
  "0123456789"
};

const uint8_t TX_times[] PROGMEM = { //for 8MHz atTiny with clk/64
  6,  // Start bit
  20, 32, 46, 60, 72, 84, 100, 110, // Data bits
  124, // Stop bit
};
const uint8_t RX_times[] PROGMEM = { //for 8MHz atTiny with clk/64
  13,  // Start bit
  26, 39, 53, 66, 78, 92, 105, 117, // Data bits
  131, // Stop bit
};

void cpuclk_init();
void uart_init();
void interrupts_init();
void setup_buffer();
void eval_byte();


ISR(PCINT0_vect){
  /* Pin Change interrupt starts transfering per byte */
    /* if high to low-> new Byte */
  if(~RX_PIN&_BV(RX_NUM)){

    TX_PORT&=~_BV(TX_NUM); // TX startbit
    //I_symbol_In & _BV(0) == 0 therefore no need to record it

    I_bit = 1; //Jump to TX first databit
    OCR0A = pgm_read_byte(&(TX_times[I_bit]));
    RX_TX = TX_NUM;
    UART_mode = RX_MNEM_LEN;

    TCNT0 = pgm_read_byte(&(TX_times[0]))+1; //sync with RX startbit
    TCCR0B= _BV(CS01) | _BV(CS00); //clk/64 prescale

    PORTB|=_BV(PB0); //debug

    /* Pin Change Interrupt Disable */
    GIMSK&=~_BV(PCIE); /* Page 51 */
  }
}


ISR(TIMER0_COMPA_vect){
  if(RX_TX==TX_NUM){
    /* transmitting */
    if (I_symbol_Out & _BV(I_bit)) {
      TX_PORT|= _BV(TX_NUM);
    } else {
      TX_PORT&=~_BV(TX_NUM);
    }
    OCR0A = pgm_read_byte(&(RX_times[I_bit]));
    RX_TX = RX_NUM;

  } else if (RX_TX==RX_NUM){

    /* receiving */
    if(RX_PIN&_BV(RX_NUM)){ //high
      I_symbol_In |= _BV(I_bit);
    }

    I_bit++;
    if(I_bit<sizeof(RX_times)){
      /* continue transfer */
      OCR0A = pgm_read_byte(&(TX_times[I_bit]));
      RX_TX = TX_NUM;
    } else {
      eval_byte();
    }
  }
}




void eval_byte(){
  char byte;
  /* If endbit is high             and            startbit is low -> byte OK */
  if((I_symbol_In & _BV(9) && (~I_symbol_In & _BV(0))){
    byte = (char)((I_symbol_In >> 1)&0xFF);
  } else {/* RX_Len is reached */}


  if(UART_mode==RX_MNEM_LEN){
    packet_head_In[packet_index_In] = byte;

    if(packet_index_In==1){ //second cycle
      if(packet_head_In[0]==0x0 && packet_head_In[1]==0x1){ /* RX_AQ */
        packet_head_Out[0]=0x0; /* TX_AQ */
        packet_head_Out[1]=0x1; /* TX_AQ */
      } /* Handle things except AQ TODO */

      I_symbol_Out = _BV(9) | ((uint16_t)packet_head_Out[0] << 1); //TX_AQ Part 1
    }
    if(packet_index_In==2){
      I_symbol_Out = _BV(9) | ((uint16_t)packet_head_Out[1] << 1); //TX_AQ Part 2
    }
    if(packet_index_In==3){

      uint16_t len_In = (uint16_t)(packet_head_In[2] << 8) | (uint16_t)(packet_head_In[3]);
      uint16_t len_Out = sizeof(my_code); //+CKSUM //TODO with buffersize function

      if(packet_head_In[0]==0x0 && packet_head_In[0]==0x1){ /* RX_AQ */
        len_Out += 2; //+CKSUM
        UART_mode = RX_CKSUM;
      } /* TODO Handle things except AQ */

      packet_length = len_In + len_Out;
      packet_head_Out[2] = (uint8_t) (packet_length >> 8);
      packet_head_Out[3] = (uint8_t) (packet_length & 0xFF);

      I_symbol_Out = _BV(9) | ((uint16_t)packet_head_Out[2] << 1); //packet_legth Part 1
      /* Handle things except AQ */
    }
  } else if(UART_mode==RX_CKSUM) {

    if(packet_index_In==4){
      packet_head_In[packet_index_In] = byte;
      I_symbol_Out = _BV(9) | ((uint16_t)packet_head_Out[3] << 1); //packet_legth Part 2
    }
    if(packet_index_In==5){
      packet_head_In[packet_index_In] = byte;
      //TODO Create CKSUM
      packet_head_Out[4] = 0x0;
      packet_head_Out[5] = 0x0;
      I_symbol_Out = _BV(9) | ((uint16_t)packet_head_Out[4] << 1); //CKSUM Part 1
    }
    if(packet_index_In==6){
      push(&buffer, byte); //TODO Check of still correct
      I_symbol_Out = _BV(9) | ((uint16_t)packet_head_Out[5] << 1); //CKSUM Part 1
      UART_mode = RX_PAYLOAD;
    }
  }

  /*Always to do*/
  packet_index_In++;
  I_symbol_In = 0;
  I_bit = 0;
  /* Pin Change Interrupt Enable */
  GIMSK|=_BV(PCIE); /* Page 51 */


  if(UART_mode==RX_PAYLOAD){

    if(/*packet_index_In - 4 or -6 because of CKSUM < packet_legth* -> not finished yet*/){
      /* Since the Length was calculated, the buffer should never run out */
      I_symbol_Out = _BV(9) | ((uint16_t)pop(&buffer) << 1);
      TCCR0B = 0;
      OCR0A = pgm_read_byte(&(TX_times[0]));
      RX_TX = TX_NUM;
      TCNT0 = 0;
    } else { /* Done */
      TCCR0B= 0; //Stop transfer
      setup_buffer(); // prepare bufffer for nexter transfer
      I_symbol_Out = 1;
      PORTB&=~_BV(PB0); //debug
      for(int i=0;i<packet_head_In;i++) packet_head_In[i]=0;
      for(int i=0;i<packet_head_Out;i++) packet_head_Out[i]=0;
      packet_index_In = 0;
      packet_length = 0;

      return;
    }
    if(/*packet_index_In - 4 < RX_len* -> bytes incoming*/){
      push(&buffer, byte);
    } else { /* No Pin Change Interrupt */
      _delay_us(65); //time between bytes
      TCCR0B = _BV(CS01) | _BV(CS00); //clk/64 prescale
    }
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
  for(int i=0;i<packet_head_In;i++) packet_head_In[i]=0;
  packet_index_In = 0;
  packet_length = 0;
  I_bit = 0;
  I_symbol_Out = 1; //Nothing
  I_symbol_In = 0;


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
