#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>

  #include <rdbuf.h>
#endif

/* For ATtiny85 */
#define TX_DDR  DDRB
#define TX_PORT PORTB
#define TX_NUM  PB4

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_NUM  PB3

#define UA_TMR_PRESCALE_REG (_BV(CS01) | _BV(CS00))
#define UA_TMR_PRESCALE_NUM 64
#define UA_HDR_LEN (4)

#define BAUD_RATE 9600

#define UART_BITTIME(bit) (((1+bit)*F_CPU)/(2*BAUD_RATE*UA_TMR_PRESCALE_NUM))

// For F_CPU ATtiny clock and prescaler value of UA_TMR_PRESCALE_NUM
const uint8_t uart_times[] PROGMEM = {
  UART_BITTIME(0),  // Start bit
  UART_BITTIME(1), UART_BITTIME(2), UART_BITTIME(3), UART_BITTIME(4), // Data bits
  UART_BITTIME(5), UART_BITTIME(6), UART_BITTIME(7), UART_BITTIME(8),
  UART_BITTIME(9), // Stop bit
};

struct {
  struct rdbuf_t buf;
  uint16_t passive_len; /* Number of bytes to stay in
                           passively clocked mode for */
  uint16_t active_len; /* Number of bytes to stay in
                          actively clocked mode for */
  uint8_t bitnum;
  uint16_t packet_index; /* The current position in a
                            packet transmission */
  char packet_header_rcvd[6]; /* The received header
                                 of the previous block */
  char packet_header_send[6]; /* The header for the next
                                 block */
  /*
   * The structure below encodes the current
   * state the software UART is in.
   *  transmission  - is set when bytes are being transfered right now
   *                  e.g. the previous block in the chain transmits a packet
   *                  or the block is sending bytes without a previous block
   *  active_clock  - is set when the previous block does not send
   *                  packets but the block should nevertheless send bytes.
   *                  The block is then itself responsible for timing the bytes.
   *  forward       - is set, when for every received byte a byte should be
   *                  transmitted.
   *                  e.g. when forwarding a aquisition request
   *  rcving_header - is set while receiving header from previous block
   *  snding_header - is set while sending header
   *                  this is nessessary because snding_header takes two bytes
   *                  longer than rcving_header. It makes the code much more
   *                  comprehensible
   */
  struct {
    uint8_t transmission : 1;
    uint8_t active_clock : 1;
    uint8_t forward : 1;
    uint8_t rcving_header : 1;
    uint8_t snding_header : 1;
  } flags;
} uart_status;

ISR(PCINT0_vect)
{
  if (RX_PIN & _BV(RX_NUM)) {
    /*
     * Verify that pin is low.
     * Search for #USBSerialConvertersSuck to find
     * out why this is nessessary.
     * Also upon startup the pin status might be unkown
     */

    return;
  }

  // Reset and start the bit timer
  // And enable the overflow interrupt
  OCR0A= pgm_read_byte(&uart_times[0]);
  TIMSK|= _BV(OCIE0A); /* Timer/Counter0 Output Compare Match A Interrupt Enable */
  TCNT0= 0;
  TCCR0B= UA_TMR_PRESCALE_REG; /* start timer */

  // This is a dirty fix to make sure the stop bit
  // is sent even if the sender is in a hurry and sends the
  // next start bit a bit early #USBSerialConvertersSuck
  TX_PORT|= _BV(TX_NUM);

  // Disable Pin change interrupts while the transmission
  // is active
  GIMSK&= ~_BV(PCIE);

  if (!uart_status.flags.transmission) {
    // This is not a byte expected by an active
    // transmission. So start one
    /* TODO fill buffer with this block machinecode*/
    uart_status.flags.transmission= 1;
    uart_status.flags.active_clock= 0;
    uart_status.flags.forward= 0;
    uart_status.flags.rcving_header= 1;
    uart_status.flags.snding_header= 0;

    uart_status.packet_index = 0;
    for(int i=0;i<6;i++) uart_status.packet_header_rcvd[i] = 0;
    for(int i=0;i<6;i++) uart_status.packet_header_send[i] = 0;
    uart_status.passive_len= UA_HDR_LEN;

  }
  else if (uart_status.packet_index<=1) {}  /* Not enough data to do anything */
  else if (uart_status.flags.transmission && uart_status.flags.snding_header) {

    if (uart_status.packet_index==2) {
      /* eval Mnemonic */
      if (uart_status.packet_header_rcvd[0]==0x0 && uart_status.packet_header_rcvd[0]==[1]==0x1) { /* rcvd AQ */
        uart_status.packet_header_send[0]=0x0 /* send AQ */
        uart_status.packet_header_send[1]=0x1 /* send AQ */
        uart_status.flags.snding_header= 1; 
        uart_status.flags.forward= 1;
      }
      /* TODO Handle things except AQ #Leonard */
    }
    if(uart_status.packet_index==4){
      /* eval packet length */
      uart_status.passive_len
      uart_status.active_len = (uint16_t)(uart_status.packet_header_rcvd[2] << 8) | (uint16_t)(uart_status.packet_header_rcvd[3]);
      uint16_t total_len = passive_len + active_len; /* If AQ, CKSUM_len is already in active_len */
      uart_status.packet_header_send[2] = (uint8_t) (total_len >> 8);
      uart_status.packet_header_send[3] = (uint8_t) (total_len & 0xFF);
    }
    /* Check if header is rcvd */
    if (uart_status.packet_index>=4) {
      if (uart_status.packet_header_rcvd[0]==0x0 && uart_status.packet_header_rcvd[0]==[1]==0x1) {
        /* rcvd AQ */
        if (uart_status.packet_index==6) {
          /* cksum rcvd */
          uart_status.flags.rcving_header = 0;
        }
        if (uart_status.packet_index==8) {
          /* cksum sent */
          uart_status.flags.snding_header = 0;
          /* Finally end this condition tree */
        }
      }
      else {

      }
    }
  }

  uart_status.packet_index++;
  uart_status.bitnum= 0;
}

ISR(TIMER0_COMPA_vect)
{
  static uint8_t b_send, b_rcvd= 0x00; /* rcvd==received */

  if (uart_status.bitnum == 0) { // start bit
    b_rcvd= 0x00;

    if (uart_status.flags.forward && !uart_status.flags.snding_header) {
      // Load next buffer byte
      if (rdbuf_pop(&uart_status.buf, (char *) &b_send) < 0) {
        // The buffer ran empty
        uart_status.flags.forward= 0;
      }
      else {

        // Forward the start bit if forwarding is requested
        TX_PORT&= ~_BV(TX_NUM);
      }
    }
    else if (uart_status.flags.snding_header && uart_status.flags.forward) {
      /* Handle Mnemonic, Length, optional Checksum */
      if(uart_status.packet_index > 2){
        b_send = uart_status.snding_header[uart_status.packet_index-3];
      } /* else { dont send because uart_status.flags.forward==0 } */
    }
  }
  else if (uart_status.bitnum >= 9) { // stop bit
    if (uart_status.flags.forward) {
      // Forward the stop bit if forwarding is requested
      TX_PORT|= _BV(TX_NUM);
    }

    if (RX_PIN & _BV(RX_NUM)) {
      /* TODO What if the stopbit was passed due to #USBSerialConvertersSuck? */
      // verify that a correct stop bit was received

      if (!uart_status.flags.rcving_header) {
        /* This is the reason why two seperate headerflags are nessessary */
        /* The header is not meant for the buffer */
        if (rdbuf_push(&uart_status.buf, b_rcvd) < 0) {
          // The buffer is full. This should not happen

          uart_status.flags.transmission= 0;

          return;
        }
      }
      else {
        uart_status.packet_header_rcvd[packet_index-1] = b_rcvd;
      }
    }

    /* Disable Timer/Counter0 Output Compare Match A Interrupt */
    TIMSK&= ~_BV(OCIE0A);
  }
  else { // data bit
    b_rcvd>>=1;

    if (RX_PIN & _BV(RX_NUM)) {
      // Sender sent a high bit
      b_rcvd|= _BV(7);
    }

    if (uart_status.flags.forward) {
      // Forward the data bit if forwarding is requested

      if (b_send & 0x01) TX_PORT|=  _BV(TX_NUM);
      else               TX_PORT&= ~_BV(TX_NUM);

      b_send>>=1;
    }

    if (uart_status.bitnum == 8) {
      /*
       * #USBSerialConvertersSuck
       * The sender might decide to send the next
       * start bit while the receiver is still
       * waiting for the stop bit.
       * To deal with this case the pin change interrupt is
       * re-enabled when the last data bit was received.
       * It might trigger because of a transition from the
       * last bit to the stop bit but that case should be filtered
       * out by the line state check at the beginning of the interrupt
       * handler
       */

      GIMSK|= _BV(PCIE);
    }
  }

  // Shedule next bit
  uart_status.bitnum++;
  OCR0A= pgm_read_byte(&uart_times[uart_status.bitnum]);
}

void uart_init(void)
{
  uart_status.flags.transmission= 0;

  // Set TX pin to driven high state
  TX_PORT&= ~_BV(TX_NUM);
  TX_DDR|= _BV(TX_NUM);

  /* Pin Change Interrupt Enable */
  PCMSK|= _BV(RX_NUM);
  GIMSK|= _BV(PCIE);
}
