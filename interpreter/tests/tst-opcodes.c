#include <stdint.h>
#include <stdio.h>
#include <minunit.h>

int tests_run = 0;

#define __UNIT_TEST__
#define main main_orig

#define PROGMEM

typedef void (fktptr_t)(void);
inline fktptr_t *pgm_read_fktptr(const void *orig)
{
  // Yiiieks
  return ((fktptr_t *)(uint64_t)orig);
}



#include "../opcodes.c"
; // Randomly placed semicolon for extra fun
#include "../vm.c"
#include "../register.c"

#undef main

const struct mem_slot mem_map[MEM_LEN]= {
  { NULL, NULL},
  { NULL, NULL},
  { NULL, NULL},
  { NULL, NULL},
  { NULL, NULL},
  { NULL, NULL},
  { NULL, NULL},
  { NULL, NULL},
};

static char *test_nothing()
{
  mu_assert("Ayyy crap", 1==1);
 
  return 0;
}


static char *all_tests() {
  mu_run_test(test_nothing);

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

