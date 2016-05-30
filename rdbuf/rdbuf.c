#include <avr/io.h>

#include "rdbuf.h"



void rdbuf_init (struct rdbuf_t *buf)
{
  buf->rdpos= 0;
  buf->wrpos= 0;

  buf->resv.resvpos = 0;
  buf->resv.f_resvbyte = 0;
  buf->resv.l_resvbyte = 0;
  buf->resv.resv_len = 0;
  buf->resv.f.resv = 0;
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

  if (buf->wrpos==buf->resv.f_resvbyte && buf->resv.f.resv) {
    /* If this happens - lets hope it doesnt -
     * the wrpos did a full round while the reservation
     * didnt finish */
    buf->wrpos = buf->resv.l_resvbyte;
  }

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


int8_t rdbuf_reserve (struct rdbuf_t *buf, uint8_t count)
{
  if(buf->resv.f.resv){
    /* Already a reservation */
    return(-1);
    /* TODO think about if there is a better solution
     * This shouldnt even be nessessary */
  }
  if(RDBUF_LEN - rdbuf_len(buf) < count){
    /* Not enough space */
    return (-2);
  }

  /* init resv */
  buf->resv.f.resv = 1;
  buf->resv.resv_len = count;

  /* reservation starts, where wrpos has been */
  buf->resv.f_resvbyte = buf->wrpos;
  buf->resv.resvpos = resv.f_resvbyte;
  buf->resv.l_resvbyte = (buf->wrpos+count<RDBUF_LEN) ?
  buf->wrpos+count : buf->wrpos+count - RDBUF_LEN;

  /* put wrpos behind reservation */
  buf->wrpos = buf->resv.l_resvbyte;

  return (0);
}


int8_t rdbuf_reserve_push (struct rdbuf_t *buf, char val)
{
  if(!buf->resv.f.resv){
    /* No reservation */
    return (-1);
  }

  buf->buf[buf->resv.resvpos]= val;

  // If the buffer length is not a power of two
  // this will break;
  buf->resv.resvpos = (buf->resv.resvpos+1) & (RDBUF_LEN-1);

  if(buf->resv.resvpos==buf->resv.l_resvbyte){
    /* reservation filled */
    buf->resv.f.resv = 0;
    buf->resv.resv_len = 0;
    buf->resv.f_resvbyte = 0;
    buf->resv.resvpos = 0;
    buf->resv.l_resvbyte = 0;
  }

  return (0);
}
