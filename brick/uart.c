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

// For 8MHz ATtiny clock and prescaler value of 64
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
  uint8_t bitnum;

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
   *  header        - is set, when the packet length is jet to be determined
   *                  e.g. not enough bytes of the header where received yet.
   */
  struct {
    uint8_t transmission : 1;
    uint8_t active_clock : 1;
    uint8_t forward : 1;
    uint8_t header : 1;
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
  TIMSK|= _BV(OCIE0A);
  TCNT0= 0;
  TCCR0B= UA_TMR_PRESCALE_REG;

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

    uart_status.flags.transmission= 1;
    uart_status.flags.active_clock= 0;
    uart_status.flags.forward= 0;
    uart_status.flags.header= 1;

    uart_status.passive_len= UA_HDR_LEN;
  }

  uart_status.bitnum= 0;
}

ISR(TIMER0_COMPA_vect)
{
  static uint8_t b_send, b_rcvd= 0x00;

  if (uart_status.bitnum == 0) { // start bit
    b_rcvd= 0x00;

    if (uart_status.flags.forward) {
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
  }
  else if (uart_status.bitnum >= 9) { // stop bit
    if (uart_status.flags.forward) {
      // Forward the stop bit if forwarding is requested
      TX_PORT|= _BV(TX_NUM);
    }

    if (RX_PIN & _BV(RX_NUM)) {
      // verify that a correct stop bit was received

      if (rdbuf_push(&uart_status.buf, b_rcvd) < 0) {
        // The buffer is full. This should not happen

        uart_status.flags.transmission= 0;

        return;
      }
    }

    // Disable ths interrupt
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

  // Enable pin change interrupt on RX pin
  PCMSK|= _BV(RX_NUM);
  GIMSK|= _BV(PCIE);
}
