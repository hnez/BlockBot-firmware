#pragma once

#include <rdbuf.h>
#include <string.h>

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
#define UA_HDR_LEN (4) // Type, length
#define UA_AQHDR_LEN (UA_HDR_LEN+2) // + Checksum
#define UA_BYTE_GAP_TIME UART_BITTIME(4) // About half a byte of delay


#define PING_TMR_PRESCALE_REG (_BV(CS02) | _BV(CS00)) // Not in use yet
#define PING_TMR_PRESCALE_NUM 1024

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
  uint16_t active_len;  /* Number of bytes to stay in
                          actively clocked mode for */
  uint16_t total_len;   /* guess it */
  uint8_t bitnum;
  uint16_t pkt_index; /* The current position in a
                            packet transmission */
  char pkt_header_rcvd[UA_AQHDR_LEN]; /* The received header
                                          of the previous block */
  char pkt_header_send[UA_AQHDR_LEN]; /* The header for the next
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
} uart;


void uart_init(void);
