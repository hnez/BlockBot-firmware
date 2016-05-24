#pragma once

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "memory.h"

#define VM_NUM_REGS 3
#define VM_NUM_STACKSLOTS 6

#define VM_ERR 0
#define VM_OK 1

#define VM_JMP_BWD 0
#define VM_JMP_FWD 1


struct vm_status_t {
  uint8_t regs[VM_NUM_REGS];
  uint8_t mem[MEM_RAMSLOTS];

  uint16_t pc;

  struct {
    uint8_t overflow : 1;
  } flags;

  struct {
    struct {
      uint16_t pc;
      uint8_t rc;
    } slots[VM_NUM_STACKSLOTS];
    uint8_t len;
    uint16_t shadow_pc;
  } jstack;

  uint16_t prog_len;
  uint8_t *prog;
};

/*
 * Update the program counter with a relative jump address while
 * checking for program counter overflows.
 * Returns VM_OK if the jump fits into the program boundaries or
 * VM_ERR otherwise.
 * The offsets for FWD jumps (0) and BWD jumps (2) are due to the fact,
 * that the program counter is updated to the next address before the
 * current operation is executed, an that length 0 jumps do not make sense.
 */
inline int8_t vm_rel_jump (struct vm_status_t *vm, uint8_t dir, uint16_t len)
{
  if (dir == VM_JMP_FWD) {
    return (__builtin_add_overflow(vm->pc, len , &vm->pc) ? VM_ERR : VM_OK);
  }
  else {
    return (__builtin_sub_overflow(vm->pc, len + 2, &vm->pc) ? VM_ERR : VM_OK);
  }
}

/*
 * Get next instruction/program space byte into *op.
 * And increment program counter _afterwards_.
 * returns VM_OK in case of success. Or a VM_ERR for error.
 */
inline int8_t vm_next_op (struct vm_status_t *vm, uint8_t *op)
{
  if (vm->pc < vm->prog_len) {
    *op= vm->prog[vm->pc];

    vm->pc++;

    return (VM_OK);
  }
  else {
    return (VM_ERR);
  }
}

uint8_t vm_step (struct vm_status_t *vm);
void vm_run (struct vm_status_t *vm);
