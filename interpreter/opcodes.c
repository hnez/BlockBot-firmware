#include "opcodes.h"
#include "memory.h"
#include "register.h"

#define op_dec_rega(o) ((o&0x0c) >> 2)
#define op_dec_regb(o) (o&0x03)
#define op_dec_reg op_dec_regb

#define op_dec_jumpa(o) (o&0x0f)

#define op_dec_mema(o) ((o&0x1c) >> 2)

static uint8_t op_short(uint8_t);
static uint8_t op_immediate(uint8_t);
static uint8_t op_jfw(uint8_t);
static uint8_t op_jbw(uint8_t);

static uint8_t op_lda(uint8_t);
static uint8_t op_lda(uint8_t);
static uint8_t op_sta(uint8_t);
static uint8_t op_sta(uint8_t);

static uint8_t op_mov(uint8_t);
static uint8_t op_or(uint8_t);
static uint8_t op_and(uint8_t);
static uint8_t op_xor(uint8_t);

static uint8_t op_add(uint8_t);
static uint8_t op_sub(uint8_t);
static uint8_t op_seq(uint8_t);
static uint8_t op_sne(uint8_t);

PROGMEM const op_cb_t op_opmap[16]= {
  op_short,
  op_immediate,
  op_jfw,
  op_jbw,

  op_lda,
  op_lda,
  op_sta,
  op_sta,

  op_mov,
  op_or,
  op_and,
  op_xor,

  op_add,
  op_sub,
  op_seq,
  op_sne
};

static uint8_t op_short (uint8_t op)
{
  switch (op & 0xfc) {
  case 0x00:
    // NOP

    return (OP_OK);
    break;

  case 0x04:
    // LD

    // Check if next prog byte is still in the program
    if (vm_verify_jmp(VM_JMP_FWD, 1)) {
      // Get the next program byte
      // Decode register number
      // Store register value and pass through the error status
      
      return (reg_set(op_dec_reg(op), vm_next_op()));
    }
    else {
      // There is no byte to read

      return (OP_ERR);
    }
    
    break;

  default:
    // Can be removed when all subinstruction bits are in use
    return (OP_ERR);
  }
}

static uint8_t op_immediate (uint8_t op)
{
  register uint8_t regnum= op_dec_reg(op);
  register uint8_t value= reg_get(regnum); 
  
  switch (op & 0xfc) {
  case (0x10): // DEC
    return(reg_set(regnum, 0 , val-1));

    break;

  case (0x14): // INC
    return(reg_set(regnum, 0 , val+1));

    break;

  case (0x18): // NOT
    return(reg_set(regnum, 0 , ~val));

    break;

  default:
    // Can be removed when all subinstruction bits are in use

    return (OP_ERR);
    break;
  }
}

static uint8_t op_jfw (uint8_t op)
{
  return (vm_do_jmp(VM_JMP_FWD, op_dec_jumpa(op)));
}

static uint8_t op_jbw (uint8_t op)
{
  return (vm_do_jmp(VM_JMP_BWD, op_dec_jumpa(op)));
}

static uint8_t op_lda (uint8_t op)
{
  register uint8_t val= mem_get(op_dec_mema(op));

  return (reg_set(op_dec_reg(op), 0 val));
}

static uint8_t op_sta (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_reg(op));

  return (mem_set(op_dec_mema(op), val));
}

static uint8_t op_mov (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_regb(op));

  return (reg_set(op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_or (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_rega(op));
  val |= reg_get(op_dec_regb(op));
  
  return (reg_set(op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_and (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_rega(op));
  val &= reg_get(op_dec_regb(op));
  
  return (reg_set(op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_xor (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_rega(op));
  val ^= reg_get(op_dec_regb(op));
  
  return (reg_set(op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_add (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_rega(op));
  val += reg_get(op_dec_regb(op));
  
  return (reg_set(op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_sub (uint8_t op)
{
  register uint8_t val= reg_get(op_dec_rega(op));
  val -= reg_get(op_dec_regb(op));

  return (reg_set(op_dec_rega(op),
                  op_dec_regb(op),
                  val));
}

static uint8_t op_seq (uint8_t op)
{
  register uint8_t vala= reg_get(op_dec_rega(op));
  register uint8_t valb= reg_get(op_dec_regb(op));

  if (vala == valb) {
    return (vm_do_jmp(VM_JMP_FWD, 1));
  }
  else {
    return (OP_OK);
  }
}

static uint8_t op_snq (uint8_t op)
{
  register uint8_t vala= reg_get(op_dec_rega(op));
  register uint8_t valb= reg_get(op_dec_regb(op));

  if (vala != valb) {
    return (vm_do_jmp(VM_JMP_FWD, 1));
  }
  else {
    return (OP_OK);
  }
}
