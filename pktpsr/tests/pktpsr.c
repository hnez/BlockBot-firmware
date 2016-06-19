#include <stdint.h>
#include <stdio.h>
#include <minunit.h>

#include <inttypes.h>

int tests_run=0;

char v_eeprom[108] = {0x01, 0x00, 0x00, 0x32, 0x01, 0x01, 0x00, 0x07, 0x46, 0x77,
                      0x64, 0x20, 0x43, 0x54, 0x43, 0x01, 0x03, 0x00, 0x04, 0x00,
                      0x00, 0x00, 0x0f, 0x01, 0x02, 0x00, 0x1b, 0x09, 0x40, 0x0a,
                      0x01, 0xe1, 0xe5, 0xcb, 0x6e, 0xb3, 0x32, 0x49, 0x92, 0xe2,
                      0xe6, 0x0b, 0x3c, 0xf3, 0xd3, 0xa3, 0x31, 0xb1, 0x23, 0xe1,
                      0x44, 0x38, 0xe0, 0xe4};
//108 chars
#define EEPROM_SPACE 108 /* bytes */

#define _BV(v) (1<<v)

#define PROGMEM

#define eeprom_read_byte(index) v_eeprom[index]


#define __UNIT_TEST__
#define main main_orig

#include "../pktpsr.c"

#include <stdlib.h>
#include <strings.h>
#include <string.h> //??

#undef main


static char *test_nth_pkt_by_index()
{
  //Finding BRICK_CONT
  mu_assert("nth_pkt_by_index: Didnt find the first packet", nth_pkt_by_index(0,EEPROM_SPACE,1)==0);

  //Finding sub_packets
  mu_assert("nth_pkt_by_index: Didnt find BRICK_NAME", nth_pkt_by_index(EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,1)==4);
  mu_assert("nth_pkt_by_index: Didnt find BRICK_PREP", nth_pkt_by_index(EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,2)==15);
  mu_assert("nth_pkt_by_index: Didnt find BRICK_BC", nth_pkt_by_index(EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,3)==23);

  //Same results // 0==brick_cont_index
  mu_assert("nth_subpkt_by_index: Didnt finde BRICK_NAME", nth_subpkt_by_index(0, 1)==4);
  mu_assert("nth_subpkt_by_index: Didnt finde BRICK_PREP", nth_subpkt_by_index(0, 2)==15);
  mu_assert("nth_subpkt_by_index: Didnt finde BRICK_BC", nth_subpkt_by_index(0, 3)==23);

  //indended Errors
  mu_assert("nth_pkt_by_index: Found sth. not existing", nth_pkt_by_index(EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,4)==-1);
  mu_assert("nth_subpkt_by_index: Found sth. not existing", nth_subpkt_by_index(0, 4)==-1);


  return (0);
}

static char *test_nth_pkt_by_type()
{
  //Finding BRICK_CONT
  mu_assert("nth_pkt_by_type: Didnt find the first packet", nth_pkt_by_type(BRICK_CONT,0,EEPROM_SPACE,1)==0);

  //indended Error
  mu_assert("nth_pkt_by_type: Found a second BRICK_CONT", nth_pkt_by_type(BRICK_CONT,0,EEPROM_SPACE,2)==-1);

  //Finding sub_packets
  mu_assert("nth_pkt_by_type: Didnt find BRICK_NAME", nth_pkt_by_type(BRICK_NAME, EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,1)==4);
  mu_assert("nth_pkt_by_type: Didnt find BRICK_PREP", nth_pkt_by_type(BRICK_PREP, EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,1)==15);
  mu_assert("nth_pkt_by_type: Didnt find BRICK_BC", nth_pkt_by_type(BRICK_BC, EEPROM_HDR_LEN,EEPROM_SPACE-EEPROM_HDR_LEN,1)==23);

  //Same results // 0==brick_cont_index
  mu_assert("nth_subpkt_by_type: Didnt finde BRICK_NAME", nth_subpkt_by_type(BRICK_NAME, 0, 1)==4);
  mu_assert("nth_subpkt_by_type: Didnt finde BRICK_PREP", nth_subpkt_by_type(BRICK_PREP, 0, 1)==15);
  mu_assert("nth_subpkt_by_type: Didnt finde BRICK_BC", nth_subpkt_by_type(BRICK_BC, 0, 1)==23);

  //indended Error
  mu_assert("nth_subpkt_by_type: Found a second BRICK_BC", nth_subpkt_by_type(BRICK_BC, 0, 2)<0);

  return (0);
}

static char *test_brick_cont_len_without_prep()
{
  mu_assert("brick_cont_len_without_prep length is wrong",
                brick_cont_len_without_prep((uint16_t)nth_pkt_by_type(BRICK_CONT, 0, EEPROM_SPACE, 1))==42);

  return (0);
}

static char *all_tests() {
  mu_run_test(test_nth_pkt_by_index);
  mu_run_test(test_nth_pkt_by_type);
  mu_run_test(test_brick_cont_len_without_prep);

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
