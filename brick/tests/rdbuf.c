#include <stdint.h>
#include <stdio.h>
#include <minunit.h>

#include <stdlib.h>
#include <strings.h>
#include <string.h> //??




#define RDBUF_LEN 32
typedef uint16_t rdbufidx_t;

struct rdbuf_t
{
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

void rdbuf_init (struct rdbuf_t *buf)
{
  buf->rdpos= 0;
  buf->wrpos= 0;

  buf->resv.f_byte = 0;
  buf->resv.l_byte = 0;
  buf->resv.len = 0;
  buf->resv.f.resv = 0;
}

uint16_t rdbuf_len(struct rdbuf_t *buf)
{
  uint16_t len= (buf->wrpos >= buf->rdpos) ?
      (uint16_t)(buf->wrpos - buf->rdpos) : (uint16_t)(RDBUF_LEN - buf->rdpos + buf->wrpos);
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

int8_t rdbuf_reserve (struct rdbuf_t *buf, uint16_t count)
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

int8_t rdbuf_put_resv (struct rdbuf_t *buf, uint16_t pos, char val)
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



struct rdbuf_t buf;

static char *test_init()
{
  mu_assert("wrpos not 0", buf.wrpos == 0);
  //...

  mu_assert("len not 0", rdbuf_len(&buf)==0);
  return (0);
}


static char *test_push1()
{
  uint16_t curlen = rdbuf_len(&buf);

  mu_assert("rdbuf_push error", rdbuf_push(&buf, '1')>=0);
  mu_assert("len didnt increase after rdbuf_push", curlen + 1 == rdbuf_len(&buf));
  return (0);
}

static char *test_pop1()
{
  uint16_t curlen = rdbuf_len(&buf);
  char val;
  mu_assert("rdbuf_pop error", rdbuf_pop(&buf, &val)>=0);
  mu_assert("len didnt decrease after rdbuf_pop", curlen - 1 == rdbuf_len(&buf));
  return (0);
}

// #define EEPROM_HDR_LEN 4
// #define AQ_HDR_LEN 6
//
// struct {
//   uint16_t index;
//   uint16_t len; /* without BRICK_PREP */
//   uint16_t total_len;
// } brick_cont;
//
// /* Hierbei bin ich eingeschlafen, das dürfte viele Logiklücken enthalten */
// static char *test_resv()
// {
//   //fake init
//   brick_cont.index = 0;
//   brick_cont.len = 12;
//   brick_cont.total_len = 20;
//   /* Reserve the precalculated len */
//   rdbuf_reserve(&buf,
//             AQ_HDR_LEN + EEPROM_HDR_LEN + brick_cont.len);
//         /*                BRICK_CONT_HDR                   */
//   mu_assert("resv didnt happen", buf.resv.f.resv);
//
//   /* init */
//   uint16_t pkt_index = brick_cont.index + EEPROM_HDR_LEN;
//   uint16_t brick_word = (uint16_t)eeprom_read_byte(pkt_index);
//   uint16_t resv_index = AQ_HDR_LEN;
//
//
//   /* fake payload */
//   for(pkt_index+= 1;
//         pkt_index < brick_cont.index + EEPROM_HDR_LEN +  brick_cont.total_len;
//         pkt_index++){
//
//     rdbuf_put_resv(&uart.buf, resv_index, 1);
//     resv_index++;
//
//   }
//
//
//   /* Create AQ header
//    * As the length of the receiving paket must be known first
//    * this happens at last */
//   rdbuf_put_resv(&uart.buf, 0, 0x0); /* AQ1 */
//   rdbuf_put_resv(&uart.buf, 1, 0x1); /* AQ2 */
//   uint16_t aq_len = brick_cont.len +
//   (uint16_t)(uart.pkt_header_rcvd[2] << 8) | (uint16_t)(uart.pkt_header_rcvd[3])
//       + 2; /* CKSUM */
//   rdbuf_put_resv(&uart.buf, 2, (uint8_t)(8 >> aq_len));
//   rdbuf_put_resv(&uart.buf, 3, (uint8_t)(aq_len&0xFF));
//
//   dbuf_put_resv(&uart.buf, 4, 0x0);
//   dbuf_put_resv(&uart.buf, 5, 0x0);
//
//   rdbuf_fin_resv(&buf);
//
// }

static char *all_tests()
{
  mu_run_test(test_init);

  mu_run_test(test_push1);
  mu_run_test(test_pop1);

//  mu_run_test(test_resv);

  return 0;
}

int main (__attribute__((unused)) int argc, __attribute__((unused)) char **argv)
{
  char *result = all_tests();

  if (result != 0) {
    printf("%s\n", result);
  }
  else {
    printf("ALL TESTS PASSED\n");
  }

  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
