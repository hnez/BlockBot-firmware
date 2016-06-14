#include <stdint.h>

#include "parse.h"

uint8_t ps_feed_hdr (struct ps_parser *ps, uint8_t byte)
{
  switch (ps->stack[ps->stack_pos].feed_len) {
  case (3):
    ps->header.type= (byte << 8);
    break;

  case (2):
    ps->header.type|= byte;
    break;

  case (1):
    ps->header.len= (byte << 8);
    break;

  case (0):
    ps->header.len|= byte;
    break;
  }

  if (!ps->stack[ps->stack_pos].feed_len) {
    ps_feedcb_t *newfeed;
    uint8_t is_container= 0;

    newfeed= ps_find_feed(ps->header.type, &is_container);

    if (is_container) {
      if (ps->stack_pos++ >= PS_MAX_DEPTH) {
        return (PS_ERR);
      }

      ps->stack[ps->stack_pos].feed_len= 4;
      ps->stack[ps->stack_pos].feed= ps_feed_hdr;
    }
    else {
      ps->stack[ps->stack_pos].feed_len= ps->header.len;
      ps->stack[ps->stack_pos].feed= newfeed;
    }
  }

  return (PS_OK);
}

void ps_start (struct ps_parser *ps)
{
  ps->stack_pos= 0;
  ps->stack[ps->stack_pos].feed_len= 0;
}

uint8_t ps_feed (struct ps_parser *ps, uint8_t byte)
{
  ps_feedcb_t *feed;

  for (uint8_t i=0; i<=ps->stack_pos; i++) {
    if (!ps->stack[i].feed_len) {
      ps->stack[i].feed= ps_feed_hdr;
      ps->stack[i].feed_len=4;

      ps->stack_pos= i;
    }

    ps->stack[i].feed_len--;
  }

  feed= ps->stack[ps->stack_pos].feed;

  return (feed ? feed(ps, byte) : PS_OK);
}
