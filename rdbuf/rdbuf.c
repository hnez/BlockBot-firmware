#include <avr/io.h>

#include "rdbuf.h"



void rdbuf_init (struct rdbuf_t *buf)
{
  buf->rdpos= 0;
  buf->wrpos= 0;

  buf->resv.f_byte = 0;
  buf->resv.l_byte = 0;
  buf->resv.len = 0;
  buf->resv.f.resv = 0;
}


uint8_t rdbuf_len(struct rdbuf_t *buf)
{
  uint8_t len= (buf->wrpos >= buf->rdpos) ?
      buf->wrpos - buf->rdpos : RDBUF_LEN - buf->rdpos + buf->wrpos;
      /* The reservation should be counted aswell, since it
      shifts the wrpos */

  return (len);
}

int8_t rdbuf_push (struct rdbuf_t *buf, char val)
{
  if (rdbuf_len(buf) == RDBUF_LEN-1) {
    // Buffer full.
    return (-1);
  }
  if (buf->wrpos==buf->resv.f_byte && buf->resv.f.resv) {
    /* If this happens the wrpos did a full round
     * while the reservation didnt finish.
     * This shouldnt happen */
    return (-2);
  }

  buf->buf[buf->wrpos]= val;

  buf->wrpos= buf->wrpos < RDBUF_LEN -1 ?
      buf->wrpos + 1 : 0;

  // return success
  return (0);
}


int8_t rdbuf_pop (struct rdbuf_t *buf, char *val)
{
  if (!rdbuf_len(buf)) {
    /* Buffer empty */
    return (-1);
  }

  if(buf->rdpos!=buf->resv.f_byte || !buf->resv.f.resv){
    /* Normal case */
    *val = buf->buf[buf->rdpos];
    buf->rdpos = buf->rdpos < RDBUF_LEN -1 ?
        buf->rdpos + 1 : 0;

    // return success
    return (0);
  }
  else { /* {buf->rdpos==buf->resv.f_byte && buf->resv.f.resv} */
    /* rdpos hit resv */
    return (-2);
  }
}


int8_t rdbuf_reserve (struct rdbuf_t *buf, uint8_t count)
{
  if(buf->resv.f.resv){
    /* Already a reservation */
    return(-1);
    /* This should never happen */
  }

  if(RDBUF_LEN - rdbuf_len(buf) < count){
    /* Not enough space */
    return (-2);
  }

  /* init resv */
  buf->resv.f.resv = 1;
  buf->resv.len = count;

  /* reservation starts, where wrpos has been */
  buf->resv.f_byte = buf->wrpos;
  buf->resv.l_byte = (buf->wrpos+count<RDBUF_LEN) ?
  buf->wrpos+count : buf->wrpos+count - RDBUF_LEN;

  /* put wrpos behind reservation */
  buf->wrpos = buf->resv.l_byte;

  return (0);
}

int8_t rdbuf_fin_resv (struct rdbuf_t *buf)
{
  if(!buf->resv.f.resv){
    /* No reservation */
    return (-1);
  }


  buf->resv.f.resv = 0;
  /*
   * Not nessessary since they will be resetted
   * when rdbuf_reserve gets called again
   * #savecycles
  buf->resv.len = 0;
  buf->resv.f_byte = 0;
  buf->resv.l_byte = 0;
  */

  return (0);
}

int8_t rdbuf_put_resv (struct rdbuf_t *buf, uint8_t pos, char val)
{
  if(!buf->resv.f.resv){
    /* No reservation */
    return (-1);
  }

  uint8_t real_pos = (pos + buf->resv.f_byte < RDBUF_LEN) ?
  buf->resv.f_byte : pos + buf->resv.f_byte - RDBUF_LEN;
  /* If pos + f_byte is more than 2 time larger than RDBUF_LEN,
   * this wont work, but that shouldnt happen
   * #savecycles */

  buf->buf[real_pos]= val;

  return (0);
}
