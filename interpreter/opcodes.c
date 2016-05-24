#include <avr/io.h>
#include <avr/pgmspace.h>

#include "opcodes.h"
#include "memory.h"
#include "register.h"

#define op_dec_rega(o) ((o&0x0c) >> 2)
#define op_dec_regb(o) (o&0x03)
#define op_dec_reg op_dec_regb

#define op_dec_jumpa(o) (o&0x0f)

#define op_dec_mema(o) ((o&0x1c) >> 2)

static uint8_t op_short (struct vm_status_t *vm, uint8_t op)
{
  switch (op & 0x0f) {
  case 0x04:
    // SOV
    if (vm->flags.overflow) {
      vm->pc++;
    }

    return (OP_OK);

  case 0x05:
    // SPU
    if (vm->jstack.len >= VM_NUM_STACKSLOTS) {
      // All stack slots are filled
      return (OP_ERR);
    }

    /* Put the program counter of the next instruction (pc is updated before
     * this instruction is called) and the value of register 4 (RC) on the stack */
    vm->jstack.slots[vm->jstack.len].pc= vm->pc;
    vm->jstack.slots[vm->jstack.len].rc= reg_get(vm, 3);

    vm->jstack.len++;

    return (OP_OK);

  case 0x06:
    // SPO
    if (vm->jstack.len == 0) {
      /* An empty jstack is not an error case as it can be
       * (ab)used to re-run the whole program*/

      vm->jstack.shadow_pc= 0;
      return (reg_set(vm, 3, 0, 0)); // Set RC to zero
    }
    else {
      vm->jstack.len--;

      vm->jstack.shadow_pc= vm->jstack.slots[vm->jstack.len].pc;
      return (reg_set(vm, 3, 0, vm->jstack.slots[vm->jstack.len].rc));
    }

  case 0x07:
    // SPJ
    vm->pc= vm->jstack.shadow_pc;

    return (OP_OK);

  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
    // LD
    uint8_t val;

    // Check if next prog byte is still in the program
    if (vm_next_op(vm, &val) == VM_OK) {
      // Get the next program byte
      // Decode register number
      // Store register value and pass through the error status

      return (reg_set(vm, op_dec_reg(op), 0, val);
    }
    else {
      // End of program ?

      return (OP_ERR);
    }

    break;

  default:
    // Can be removed when all subinstruction bits are in use
    return (OP_ERR);
  }
}

static uint8_t op_unary (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t regnum= op_dec_reg(op);
  register uint8_t value= reg_get(vm, regnum);

  switch (op & 0x0c) {
  case (0x00): // DEC
    vm->flags.overflow=
      __builtin_sub_overflow(value, 1, &value)) ? 1 : 0;

    return(reg_set(vm, regnum, 0 , value));

  case (0x04): // INC
    vm->flags.overflow=
      __builtin_add_overflow(value, 1, &value)) ? 1 : 0;

    return(reg_set(vm, regnum, 0 , value));

  case (0x08): // NOT
    return(reg_set(vm, regnum, 0 , ~value));

  case (0x0c): // SRR
    vm->flags.overflow= (value & 1) ? 1 : 0;

    return(reg_set(vm, regnum, 0 , value>>1));
  }
}

static uint8_t op_jfw (struct vm_status_t *vm, uint8_t op)
{
  return (vm_rel_jump(vm, VM_JMP_FWD, op_dec_jumpa(op)));
}

static uint8_t op_jbw (struct vm_status_t *vm, uint8_t op)
{
  return (vm_rel_jump(vm, VM_JMP_BWD, op_dec_jumpa(op)));
}

static uint8_t op_lda (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t val;

  if (mem_get(vm, op_dec_mema(op), &val) != MEM_OK) {
    return (OP_ERR);
  }

  return (reg_set(vm, op_dec_reg(op), 0, val));
}

static uint8_t op_sta (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t val= reg_get(vm, op_dec_reg(op));

  return (mem_set(vm, op_dec_mema(op), val));
}

static uint8_t op_mov (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t val= reg_get(vm, op_dec_regb(op));

  return (reg_set(vm,
                  op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_or (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t val= reg_get(vm, op_dec_rega(op));
  val |= reg_get(vm, op_dec_regb(op));

  return (reg_set(vm,
                  op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_and (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t val= reg_get(vm, op_dec_rega(op));
  val &= reg_get(vm, op_dec_regb(op));

  return (reg_set(vm,
                  op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_xor (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t val= reg_get(vm, op_dec_rega(op));
  val ^= reg_get(vm, op_dec_regb(op));

  return (reg_set(vm,
                  op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_add (struct vm_status_t *vm, uint8_t op)
{
  uint8_t rega= reg_get(vm, op_dec_rega(op));
  uint8_t regb= reg_get(vm, op_dec_regb(op));
  uint8_t val;

  vm->flags.overflow=
    __builtin_add_overflow(rega, regb, &val)) ? 1 : 0;

  return (reg_set(vm,
                  op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_sub (struct vm_status_t *vm, uint8_t op)
{
  uint8_t rega= reg_get(vm, op_dec_rega(op));
  uint8_t regb= reg_get(vm, op_dec_regb(op));
  uint8_t val;

  vm->flags.overflow=
    __builtin_sub_overflow(rega, regb, &val)) ? 1 : 0;

  return (reg_set(vm,
                  op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_seq (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t vala= reg_get(vm, op_dec_rega(op));
  register uint8_t valb= reg_get(vm, op_dec_regb(op));

  if (vala == valb) {
    vm->pc++;
  }

  return (OP_OK);
}

static uint8_t op_sne (struct vm_status_t *vm, uint8_t op)
{
  register uint8_t vala= reg_get(vm, op_dec_rega(op));
  register uint8_t valb= reg_get(vm, op_dec_regb(op));

  if (vala != valb) {
    vm->pc++;
  }

  return (OP_OK);
}

PROGMEM const op_cb_t op_opmap[16]= {
  op_short,
  op_unary,
  op_jfw,
  op_jbw,

  op_mov,
  op_or,
  op_and,
  op_xor,

  op_add,
  op_sub,
  op_seq,
  op_sne,

  op_lda,
  op_lda,
  op_sta,
  op_sta
};
