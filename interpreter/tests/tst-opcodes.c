#include <stdint.h>
#include <stdio.h>

int tests_run = 0;

#define __UNIT_TEST__
#define main main_orig

#define PROGMEM

typedef void (fktptr_t)(void);
inline fktptr_t *pgm_read_fktptr(const void *orig)
{
  /* I actually taste a bit of puke
   * whenever i read the line below.
   * But it works for me (TM)*/

  return ((fktptr_t *)*(uint64_t*)orig);
}


#include "../opcodes.c"
#include "../vm.c"
#include "../register.c"

#undef main

#include <strings.h>
#include <minunit.h>

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

static char *test_emptyprog()
{
  struct vm_status_t vm;
  uint8_t prog[8];
  bzero(&vm, sizeof(vm));
  bzero(&prog, sizeof(prog));

  vm.prog= prog;
  vm.prog_len= 0;

  mu_assert("vm_step ran an empty program",
            vm_step(&vm) == VM_ERR);

  bzero(&vm, sizeof(vm));

  return 0;
}

static char *test_op_sov()
{
  struct vm_status_t vm;
  uint8_t prog[]={0x11, 0x04, 0x16, 0x17}; // DEC RA, SOV, INC RB, INC RC
  bzero(&vm, sizeof(vm));

  vm.prog= prog;
  vm.prog_len= sizeof(prog);

  vm_run(&vm);

  mu_assert("program including SOV did not fully execute",
            vm.pc == sizeof(prog));

  mu_assert("Something went wrong in program including SOV",
            (vm.regs[0] == 0xff) && (vm.regs[1] == 0x00) && (vm.regs[2] == 0x01));

  return 0;
}

static char *test_op_stack()
{
  struct vm_status_t vm;
  uint8_t prog[]={0x44, 0x0b, 0x0a, 0x05, 0x4c, 0x15, 0x06, 0x13, 0xa3, 0x07};
  // MOV RA RZ, LD RC, 10, SPU, MOV RC RZ, INC RA, SPO, SEQ RZ RC, SPJ
  bzero(&vm, sizeof(vm));

  vm.prog= prog;
  vm.prog_len= sizeof(prog);

  vm_run(&vm);

  mu_assert("program including stack operations did not fully execute",
            vm.pc == sizeof(prog));

  mu_assert("Something went wrong in program including stack operations",
            (vm.regs[0] == 10) && (vm.regs[2] == 0));

  return 0;
}

static char *test_op_ld()
{
  struct vm_status_t vm;
  uint8_t prog[]={0x09, 0x32, 0x0a, 0x33, 0x0b, 0x34, 0x08, 0x35};
  // LD RA, 50, LD RB, 51, LD RC, 52, LD RZ, 53

  bzero(&vm, sizeof(vm));

  vm.prog= prog;
  vm.prog_len= sizeof(prog);

  mu_assert("Loading into RA threw an error",
            vm_step(&vm) == VM_OK);

  mu_assert("Loading into RA failed",
            vm.regs[0] == 50);


  mu_assert("Loading into RB threw an error",
            vm_step(&vm) == VM_OK);

  mu_assert("Loading into RB failed",
            vm.regs[1] == 51);


  mu_assert("Loading into RC threw an error",
            vm_step(&vm) == VM_OK);

  mu_assert("Loading into RC failed",
            vm.regs[2] == 52);


  mu_assert("Loading into RZ threw no error",
            vm_step(&vm) == VM_ERR);

  return 0;
}


static char *test_op_jfw()
{
  struct vm_status_t vm;
  uint8_t prog[]={0x44, 0x15, 0x24, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15};
  // MOV RA RZ, INC RA, JFW 4, INC RA, INC RA, INC RA, INC RA, INC RA, INC RA

  bzero(&vm, sizeof(vm));

  vm.prog= prog;
  vm.prog_len= sizeof(prog);

  vm_run(&vm);

  mu_assert("Something went wrong in program containing JFW",
            vm.regs[0] == 3);

  return 0;
}

static char *test_op_jbw()
{
  struct vm_status_t vm;
  uint8_t prog[]={0x09, 0x0a, 0x0a, 0x02, 0x12, 0x15, 0x15, 0x15, 0x15, 0x15, 0xa2, 0x36};
  // LD RA, 10, LD RB, 2, DEC RB, INC RA, INC RA, INC RA, INC RA, INC RA, SEQ RZ RB, JBW 6

  bzero(&vm, sizeof(vm));

  vm.prog= prog;
  vm.prog_len= sizeof(prog);

  vm_run(&vm);

  mu_assert("Something went wrong in program containing JFW",
            vm.regs[0] == 20);

  return 0;
}

static char *all_tests() {
  mu_run_test(test_emptyprog);
  mu_run_test(test_op_sov);
  mu_run_test(test_op_stack);
  mu_run_test(test_op_ld);
  mu_run_test(test_op_jfw);
  mu_run_test(test_op_jbw);

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
