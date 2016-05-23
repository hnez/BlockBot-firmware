#include <avr/io.h>

#include "rdbuf.h"

void rdbuf_init (struct rdbuf_t *buf)
{
  buf->rdpos= 0;
  buf->wrpos= 0;
}

uint8_t rdbuf_len(struct rdbuf_t *buf)
{
  uint8_t len= (buf->wrpos >= buf->rdpos) ?
    buf->wrpos - buf->rdpos : RDBUF_LEN - buf->rdpos + buf->wrpos;

  return (len);
}

int8_t rdbuf_push (struct rdbuf_t *buf, char val)
{
  if (rdbuf_len(buf) == RDBUF_LEN-1) {
    // Buffer full.
    return (-1);
  }

  buf->buf[buf->wrpos]= val;

  // If the buffer length is not a power of two
  // this will break;
  buf->wrpos= (buf->wrpos+1) & (RDBUF_LEN-1);

  // return success
  return (0);

}

int8_t rdbuf_pop (struct rdbuf_t *buf, char *val)
{
  if (!rdbuf_len(buf)) {
    // Buffer empty. Signal error
    return (-1);
  }

  *val= buf->buf[buf->rdpos];

  // If the buffer length is not a power of two
  // this will break;
  buf->rdpos= (buf->rdpos+1) & (RDBUF_LEN-1);

  // return success
  return (0);
}
