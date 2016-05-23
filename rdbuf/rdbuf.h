#pragma once

#include <avr/io.h>

// Keep the buffer length a power of two or see the world burn
#define RDBUF_LEN 32

struct rdbuf_t {
  uint8_t rdpos;
  uint8_t wrpos;

  char buf[RDBUF_LEN];
};

void rdbuf_init (struct rdbuf_t *);
uint8_t rdbuf_len(struct rdbuf_t *);
int8_t rdbuf_push (struct rdbuf_t *, char);
int8_t rdbuf_pop (struct rdbuf_t *, char *);
