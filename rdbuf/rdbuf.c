#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
#endif

#include "rdbuf.h"

void rdbuf_init (struct rdbuf_t *buf)
{
  buf->rdpos= 0;
  buf->wrpos= 0;

  buf->resv.f_byte = 0;
  buf->resv.l_byte = 0;
  buf->resv.len = 0;
  buf->f.resv = 0;
  buf->f.full = 0;
}

uint16_t rdbuf_len(struct rdbuf_t *buf)
{
  uint16_t len= (buf->wrpos >= buf->rdpos) ?
      buf->wrpos - buf->rdpos : RDBUF_LEN - buf->rdpos + buf->wrpos;
      /* The reservation should be counted aswell, since it
      shifts the wrpos */
  if (buf->f.full) len=RDBUF_LEN;
  return (len);
}

int8_t rdbuf_push (struct rdbuf_t *buf, char val)
{
  if (buf->f.full) return (-1);

  buf->buf[buf->wrpos]= val;

  buf->wrpos= buf->wrpos < RDBUF_LEN -1 ?
      buf->wrpos + 1 : 0;

  //if rdpos is reached, the buffer is full
  if(buf->wrpos==buf->rdpos) buf->f.full = 1;

  // return success
  return (0);
}

int8_t rdbuf_pop (struct rdbuf_t *buf, char *val)
{
  if (!rdbuf_len(buf)) {
    return (BUFFER_EMPTY);
  }

  if(buf->rdpos!=buf->resv.f_byte || !buf->f.resv){
    /* Normal case */
    *val = buf->buf[buf->rdpos];

    buf->f.full=0;

    buf->rdpos = buf->rdpos < RDBUF_LEN -1 ?
        buf->rdpos + 1 : 0;

    // return success
    return (0);
  }
  else { /* {buf->rdpos==buf->resv.f_byte && buf->f.resv} */
    /* rdpos hit resv */
    return (HIT_RESV);
  }
}

int8_t rdbuf_reserve (struct rdbuf_t *buf, uint16_t count)
{
  if(buf->f.resv){
    /* Already a reservation */
    return(-1);
    /* This should never happen */
  }

  if(RDBUF_LEN - rdbuf_len(buf) < count){
    /* Not enough space */
    return (-2);
  }

  /* init resv */
  buf->f.resv = 1;
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
  if(!buf->f.resv){
    /* No reservation */
    return (-1);
  }


  buf->f.resv = 0;
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

int8_t rdbuf_put_resv (struct rdbuf_t *buf, uint16_t pos, char val)
{
  if(!buf->f.resv){
    /* No reservation */
    return (-1);
  }

  uint16_t real_pos = (pos + buf->resv.f_byte < RDBUF_LEN) ?
  buf->resv.f_byte + pos : pos + buf->resv.f_byte - RDBUF_LEN;


  if (buf->resv.f_byte < buf->resv.l_byte) {
    /* out of resv space */
    if (real_pos < buf->resv.f_byte) return (-2);
    if (real_pos > buf->resv.l_byte) return (-2);
  }
  else {
    /* out of resv space */
    if (real_pos > buf->resv.f_byte && real_pos < buf->resv.l_byte) return (-2);
  }

  if (real_pos > buf->resv.l_byte && real_pos < buf->resv.f_byte) return (-3);

  buf->buf[real_pos]= val;

  return (0);
}
