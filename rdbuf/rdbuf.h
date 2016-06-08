#pragma once

#include <avr/io.h>

// Keep the buffer length a power of two or see the world burn
#define RDBUF_LEN 32

struct rdbuf_t {
  uint8_t rdpos;
  uint8_t wrpos;

  char buf[RDBUF_LEN];

  struct {
    uint8_t resvpos; /*current write position */
    uint8_t f_byte; /* First reservated byte */
    uint8_t l_byte; /* Last reservated byte */
    uint8_t len;

    struct {
      uint8_t resv : 1; /* reservation flag */
    } f; /* flags */
    /* Since only one reservation at a time is possible
     * and its binded to the buffer, why not just put it
     * in there ? */
  } resv;

};



void rdbuf_init (struct rdbuf_t *);
uint8_t rdbuf_len(struct rdbuf_t *);
int8_t rdbuf_push (struct rdbuf_t *, char);
int8_t rdbuf_pop (struct rdbuf_t *, char *);
int8_t rdbuf_reserve (struct rdbuf_t *buf, uint8_t count);
int8_t rdbuf_fin_resv (struct rdbuf_t *buf);
int8_t rdbuf_put_resv (struct rdbuf_t *buf, uint8_t pos, char val);
