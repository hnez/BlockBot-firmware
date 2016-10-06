#pragma once

#include <stdbool.h>

#include "rdbuf.h"


#define UA_HDR_LEN 4 // Type, length
#define UA_AQHDR_LEN (UA_HDR_LEN+2) // + Checksum

typedef bool (*uart_rx_cb)(uint8_t byte);
typedef bool (*uart_tx_cb)(uint8_t *byte);

struct uart_t{
  uint8_t bit_num;

  uart_tx_cb cb_tx;
  uart_rx_cb cb_rx;

  uint8_t clk_passive :1;
  uint8_t clk_active :1;
};

extern struct uart_t uart;

void uart_init(void);
void uart_start_active(void);
void uart_start_passive(void);
void uart_disabled_mode(void);

