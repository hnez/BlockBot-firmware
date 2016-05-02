#include <avr/io.h>
#include <avr/pgmspace.h>
#include "vm.h"
#include "opcodes.h"

uint8_t vm_step (void)
{
  if (vm_verify_jmp(VM_JMP_FWD, 1)) { 
    uint8_t op= vm_next_op();

    // "ISO C forbids conversion of object pointer to function pointer type"
    // op_cb_t cb= (op_cb_t)pgm_read_ptr(&op_opmap[op_dec_mayor(op)]);
    // This workaround does not feel quite right:
    op_cb_t cb= (op_cb_t)pgm_read_word(&op_opmap[op_dec_mayor(op)]);

    return (cb(op));
  }
  else {
    return (VM_ERR);
  }
}

void vm_run (void)
{
  while (vm_step() == VM_OK);
}
