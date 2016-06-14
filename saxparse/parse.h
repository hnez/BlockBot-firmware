#pragma once

#include <stdint.h>

#define PS_MAX_DEPTH 4

#define PS_OK 1
#define PS_ERR (!PS_OK)

struct ps_parser;

typedef uint8_t(ps_feedcb_t)(struct ps_parser *, uint8_t);

struct ps_parser {
  struct {
    ps_feedcb_t *feed;
    uint16_t feed_len;
  } stack[PS_MAX_DEPTH];
  uint8_t stack_pos;

  struct {
    uint16_t type;
    uint16_t len;
  } header;
};

void ps_start (struct ps_parser *);
uint8_t ps_feed (struct ps_parser *, uint8_t);

/* This function has to be provided by the
 * user of the library */
ps_feedcb_t *ps_find_feed(uint16_t type, uint8_t *is_container);
