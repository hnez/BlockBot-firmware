#pragma once



#define RDBUF_LEN 32
typedef uint16_t rdbufidx_t;


struct rdbuf_t {
  rdbufidx_t rdpos;
  rdbufidx_t wrpos;

  char buf[RDBUF_LEN];

  struct {
    rdbufidx_t resvpos; /*current write position */
    rdbufidx_t f_byte; /* First reservated byte */
    rdbufidx_t l_byte; /* Last reservated byte */
    rdbufidx_t len;

    struct {
      uint8_t resv : 1; /* reservation flag */
    } f; /* flags */
  } resv;

};


void rdbuf_init (struct rdbuf_t *);
uint8_t rdbuf_len(struct rdbuf_t *);
int8_t rdbuf_push (struct rdbuf_t *, char val);
int8_t rdbuf_pop (struct rdbuf_t *, char *val);
int8_t rdbuf_reserve (struct rdbuf_t *buf, uint16_t count);
int8_t rdbuf_fin_resv (struct rdbuf_t *buf);
int8_t rdbuf_put_resv (struct rdbuf_t *buf, uint16_t pos, char val);
