#include <stdint.h>
#include <stdio.h>
#include <minunit.h>

int tests_run=0;
struct rdbuf_t buffer;

#define _BV(v) (1<<v)
#define PROGMEM
#define pgm_read_byte(ptr) (*((uint8_t *)ptr))


#define __UNIT_TEST__
#define main main_orig

#include "../rdbuf.c"

#include <stdlib.h>
#include <strings.h>
#include <string.h> //??

#undef main

// static char *test_init()
// {
//   mu_assert("wrpos not 0", buffer.wrpos == 0);
//   //...
//
//   mu_assert("len not 0", rdbuf_len(&buffer)==0);
//   return (0);
// }


static char *test_push1()
{
  uint16_t curlen = rdbuf_len(&buffer);

  mu_assert("rdbuf_push error", rdbuf_push(&buffer, '1')>=0);
  mu_assert("len didnt increase after rdbuf_push", curlen + 1 == rdbuf_len(&buffer));
  return (0);
}

static char *test_pop1()
{
  uint16_t curlen = rdbuf_len(&buffer);
  char val;
  mu_assert("rdbuf_pop error", rdbuf_pop(&buffer, &val)>=0);
  mu_assert("len didnt decrease after rdbuf_pop", curlen - 1 == rdbuf_len(&buffer));
  mu_assert("rdbuf_pop val is not 1", val=='1');
  return (0);
}


static char *test_resv()
{

  //Byte 0, 1 occupied
  mu_assert("rdbuf_push error", rdbuf_push(&buffer, '1')>=0);
  mu_assert("rdbuf_push error", rdbuf_push(&buffer, '1')>=0);

  uint16_t rest_space = RDBUF_LEN - rdbuf_len(&buffer);

  //intential error
  mu_assert("resv didnt throw out an intential error", rdbuf_reserve(&buffer, rest_space+1)==-2);

  //resv
  mu_assert("resv didnt work out the way inteded", rdbuf_reserve(&buffer, rest_space-1)==0);

  //intential error
  mu_assert("resv didnt throw out an intential error", rdbuf_reserve(&buffer, rest_space)==-1);


  mu_assert("rdbuf_push error", rdbuf_push(&buffer, '1')>=0);
  //buffer should be full now

  //intential error
  mu_assert("rdbuf_push should be full", rdbuf_push(&buffer, '1')==-1);

  char val;
  mu_assert("rdbuf_pop error", rdbuf_pop(&buffer, &val)>=0);
  mu_assert("rdbuf_pop val is not 1", val=='1');
  mu_assert("rdbuf_pop error", rdbuf_pop(&buffer, &val)>=0);
  mu_assert("rdbuf_pop val is not 1", val=='1');

  //intential error
  mu_assert("rdbuf_pop should have hit f_byte", rdbuf_pop(&buffer, &val)==-2);


  mu_assert("rdbuf_push error", rdbuf_push(&buffer, '1')>=0);
  mu_assert("rdbuf_push error", rdbuf_push(&buffer, '1')>=0);
  //wrpos should be at f_byte now
  mu_assert("wrpos!=f_byte", buffer.wrpos==buffer.resv.f_byte);
  mu_assert("!buffer.f.resv", buffer.f.resv);

  //intential error
  mu_assert("rdbuf_push should have hit f_byte and rdpos", rdbuf_push(&buffer, '1')==-1);

  for(uint16_t i=0; i < rest_space-1; i++){
    mu_assert("rdbuf_put_resv error", rdbuf_put_resv(&buffer, i, '2')==0);
  }

  // intential error
  mu_assert("rdbuf_put_resv put a byte outside of resv_space", rdbuf_put_resv(&buffer, rest_space, '1')==-2);

  rdbuf_fin_resv(&buffer);

  for (uint16_t i=0; i < rest_space-1; i++){
    mu_assert("rdbuf_pop error", rdbuf_pop(&buffer, &val)>=0);
    mu_assert("rdbuf_pop val is not 2", val=='2');
  }

  return (0);
}

static char *all_tests()
{
  //mu_run_test(test_init);
  //ridiculous
  
  rdbuf_init(&buffer);

  for(uint8_t i=0; i<RDBUF_LEN; i++){
    mu_run_test(test_push1);
  }

  for(uint8_t i=0; i<RDBUF_LEN; i++){
    mu_run_test(test_pop1);
  }

  //buffer should be empty again

  mu_run_test(test_resv);

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
