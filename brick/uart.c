#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>


  #include <rdbuf.h>
#endif

#include <string.h> /* nessessary? */
#define bzero(dst, size) memset(dst, 0, size)

#define BAUD_RATE 9600
#define UART_BITTIME(bit) (((1+2*bit)*F_CPU)/(2L*BAUD_RATE*UA_TMR_PRESCALE_NUM))

/* For ATtiny85 */
#define TX_DDR  DDRB
#define TX_PORT PORTB
#define TX_NUM  PB4

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_NUM  PB3

#define UA_TMR_PRESCALE_REG (_BV(CS01) | _BV(CS00))
#define UA_TMR_PRESCALE_NUM 64
#define UA_BYTE_GAP_TIME UART_BITTIME 4 // About half a byte of delay


#define PING_TMR_PRESCALE_REG (_BV(CS02) | _BV(CS00)) // Not in use yet
#define PING_TMR_PRESCALE_NUM 1024

// For F_CPU ATtiny clock and prescaler value of UA_TMR_PRESCALE_NUM
const uint8_t uart_times[] PROGMEM= {
  UART_BITTIME(0),  // Start bit
  UART_BITTIME(1), UART_BITTIME(2), UART_BITTIME(3), UART_BITTIME(4), // Data bits
  UART_BITTIME(5), UART_BITTIME(6), UART_BITTIME(7), UART_BITTIME(8),
  UART_BITTIME(9), // Stop bit
};



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

  if (!uart.flags.transmission) {
    // This is not a byte expected by an active
    // transmission. So start one

    uart.flags.transmission= 1;
    uart.flags.active_clock= 0;
    uart.flags.forward= 1;
    uart.flags.rcving_header= 1;

    uart.rcvd_index= 0;
    uart.send_index= 0;

    uart.bitnum= 0;
    return;
  }
  // may check if aq later
  else if (uart.rcvd_index== 3) {
    uart.passive_len = (uint16_t)(uart.aq_hdr_rcvd[2] << 8)
                      | (uint16_t)uart.aq_hdr_rcvd[3];
  }
  else if (uart.rcvd_index==5){
    // assumes aq, may check cksum later
    uart.flags.rcving_header= 0;
  }

  /* This should be the last PC interrupt.
   * TODO check if off by one error */
  if (3 + uart.rcvd_index - 1==uart.passive_len){
    uart.flags.active_clock= 1;
  }

  uart.rcvd_index++;
  uart.bitnum= 0;
}



ISR(TIMER0_COMPA_vect)
{
  static uint8_t b_send, b_rcvd= 0x00; /* rcvd==received */


  if (uart.bitnum== 0) { // start bit
    b_rcvd= 0x00;

    if (uart.flags.forward) {
      // Load next buffer byte
      uint8_t bufstat= rdbuf_pop(&uart.buf, (char *) &b_send);

      // TODO is bufstat==BUFFER_EMPTY && !uart.flags.active_clock possible?
      if (bufstat==BUFFER_EMPTY && uart.flags.active_clock) {
        // The buffer ran empty
        uart.flags.forward= 0;
        // TODO: Transmission complete
      }
      else if (bufstat==HIT_RESV) {
        /* transmit nothing until
         * resv is finished */
        b_send= 0xFF;
      }
      else /* if (bufstat>0) */ {
        // Forward the start bit if forwarding is requested
        TX_PORT&= ~_BV(TX_NUM);
      }
    }
  }

  else if (uart.bitnum >= 9) { // stop bit
    if (uart.flags.forward) {
      // Forward the stop bit if forwarding is requested
      TX_PORT|= _BV(TX_NUM);
    }

    /* Dont verify that a correct stop bit was received
     * #USBSerialConvertersSuck */

    if (!uart.flags.rcving_header) {
       /* The header is not meant for the buffer */

      if (rdbuf_push(&uart.buf, b_rcvd) < 0) {
        /* The buffer is full. Everything will break.
         * This should not happen */
         // panic();
      }
    }
    else {
      uart.aq_hdr_rcvd[uart.rcvd_index]= b_rcvd;
      if (uart.rcvd_index >= 5) {
        uart.flags.rcving_header= 0;
      }
    }


    if(uart.flags.active_clock){
      if (rdbuf_len(&uart.buf)>0){
        // Prepare interrupt for next cycle, compare match int is still enabled
        OCR0A= pgm_read_byte(&uart_times[0]);

        /* The counter will count up to 255, then wrap around
         * to zero, reach uart_times[0] and start a new cycle */
        TCNT0= 255 - UA_BYTE_GAP_TIME;

        uart.bitnum= 0;
      }
      /* When everything is transfered */
      else {
        GIMSK|= _BV(PCIE); /* Enable Pin Change interrupt */
        TCCR0B= 0; /* Stop timer */
        /* There is no need to set anything else because everything
           is set in the next Pin Change Interrupt */
      }
    }
    else {
      /* Disable Timer/Counter0 Output Compare Match A Interrupt */
      TIMSK&= ~_BV(OCIE0A);
    }
  }
  else { // data bit
    b_rcvd>>=1;

    if (RX_PIN & _BV(RX_NUM)) {
      // Sender sent a high bit
      b_rcvd|= _BV(8);
    }

    if (uart.flags.forward) {
      // Forward the data bit if forwarding is requested

      if (b_send & 0x01) TX_PORT|=  _BV(TX_NUM);
      else               TX_PORT&= ~_BV(TX_NUM);

      b_send>>=1;
    }

    if (uart.bitnum== 8 && !uart.flags.active_clock) {
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
  uart.bitnum++;
  OCR0A= pgm_read_byte(&uart_times[uart.bitnum]);
}




void uart_init(void)
{
  uart.flags.transmission= 0;

  // Set TX pin to driven high state
  TX_PORT&= ~_BV(TX_NUM);
  TX_DDR|= _BV(TX_NUM);

  /* Pin Change Interrupt Enable */
  PCMSK|= _BV(RX_NUM);
  GIMSK|= _BV(PCIE);
}
