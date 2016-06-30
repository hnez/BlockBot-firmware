#pragma once

#define UA_HDR_LEN 4 // Type, length
#define UA_AQHDR_LEN (UA_HDR_LEN+2) // + Checksum

struct {
  struct rdbuf_t buf;
  uint16_t passive_len; /* Number of bytes to stay in
                           passively clocked mode for */

  uint16_t total_len;
  uint8_t bitnum;
  uint16_t rcvd_index;

  char hdr_rvcd[UA_AQHDR_LEN]; /* The received header
                                          of the previous block */
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
   *  rcving_header - is set, while reveiving the previous header
   */

  struct {
    uint8_t transmission : 1;
    uint8_t active_clock : 1;
    uint8_t forward : 1;
    uint8_t backward : 1;
    uint8_t rcving_header : 1;
    uint8_t ping_rcvd : 1;
    uint8_t first_brick : 1;
    uint8_t aq_complete : 1;
  } flags;
} uart;

void uart_init(void);
void communicate(void);
