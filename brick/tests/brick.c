#include <stdint.h>
#include <stdio.h>
#include <minunit.h>

#define UA_HDR_LEN 4 // Type, length
#define UA_AQHDR_LEN (UA_HDR_LEN+2) // + Checksum
#define BRICK_CONT 0x0100

#define EEPROM_HDR_LEN 4 /* Mnemonic, payload_length */
#define AQ_HDR_LEN 6 /* Mnemonic, payload_length, CKSUM */

#define BRICK_PREP 0x0103

int tests_run = 0;

struct rdbuf_t {
  uint8_t neccesary;
};

struct {
  struct rdbuf_t buf;
  char pkt_header_rcvd[UA_AQHDR_LEN];

  struct {
    uint8_t forward : 1;
  } flags;
} uart;

#define _BV(v) (1<<v)
#define ISR(vect) void vect (void)
#define PROGMEM
#define pgm_read_byte(ptr) (*((uint8_t *)ptr))
#define sei()
#define uart_init()

char v_eeprom[108] = {0x01, 0x00, 0x00, 0x32, 0x01, 0x01, 0x00, 0x07, 0x46, 0x77,
                      0x64, 0x20, 0x43, 0x54, 0x43, 0x01, 0x03, 0x00, 0x04, 0x00,
                      0x00, 0x00, 0x0f, 0x01, 0x02, 0x00, 0x1b, 0x09, 0x40, 0x0a,
                      0x01, 0xe1, 0xe5, 0xcb, 0x6e, 0xb3, 0x32, 0x49, 0x92, 0xe2,
                      0xe6, 0x0b, 0x3c, 0xf3, 0xd3, 0xa3, 0x31, 0xb1, 0x23, 0xe1,
                      0x44, 0x38, 0xe0, 0xe4};
//108 chars
#define EEPROM_SPACE 108 /* bytes */
#define eeprom_read_byte(index) v_eeprom[index]

//######################################

uint8_t rdbuf_len(struct rdbuf_t *buf)
{
  if(buf->neccesary>0){}
  return (100);
}

int8_t rdbuf_push (struct rdbuf_t *buf, char val)
{
  if(buf->neccesary>0){}
  if(val>0){}
  return (0);

}

int8_t rdbuf_pop (struct rdbuf_t *buf, char *val)
{
  if(buf->neccesary>0){}
  *val=1;
  return (0);
}

int8_t rdbuf_reserve (struct rdbuf_t *buf, uint16_t count)
{
  if(buf->neccesary>0){}
  if(count>1){}
  return (0);
}

int8_t rdbuf_put_resv (struct rdbuf_t *buf, uint16_t pos, char val)
{
  if(buf->neccesary>0){}
  if(pos==0){}
  if(val==0){}
  return (0);
}

int8_t rdbuf_fin_resv (struct rdbuf_t *buf)
{
  if(buf->neccesary>0){}
  return (0);
}


int16_t nth_pkt_by_type(uint16_t pkt, uint16_t f_index, uint16_t len, uint8_t n)
{
  if(pkt==BRICK_CONT && f_index>=0 && len>0 && n>=0){
    return (0);
  }

  return (-1);
}

int16_t brick_cont_len_without_prep(uint16_t cont_index)
{
  if(cont_index==0){
    return (42);
  }

  return (-1);
}
//######################################


#define __UNIT_TEST__
#define main main_orig

#include "../brick.c"

#include <stdlib.h>
#include <strings.h>
#include <string.h> //??

#undef main




static char *test_init_brick_cont()
{
  mu_assert("BRICK_CONT not found", init_brick_cont()==0);
  return 0;
}

static char *test_make_aq()
{
  mu_assert("make_aq Problem", make_aq()==0);
  return 0;
}


static char *all_tests()
{
  //neccesary because c doesnt allow empty structs
  uart.buf.neccesary = 1;


  mu_run_test(test_init_brick_cont);
  mu_run_test(test_make_aq);

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
