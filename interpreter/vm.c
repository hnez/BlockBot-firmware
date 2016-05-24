#include <avr/io.h>
#include <avr/pgmspace.h>
#include "vm.h"
#include "opcodes.h"

uint8_t vm_step (struct vm_status_t *vm)
{
  uint8_t op;

  if (vm_next_op(vm, &op) == VM_OK) {

    // "ISO C forbids conversion of object pointer to function pointer type"
    //  op_cb_t cb= (op_cb_t)pgm_read_ptr(&op_opmap[op_dec_mayor(op)]);
    // This workaround does not feel quite right:
    op_cb_t cb= (op_cb_t)pgm_read_word(&op_opmap[op_dec_mayor(op)]);

    return (cb(vm, op));
  }
  else {
    return (VM_ERR);
  }
}

void vm_run (struct vm_status_t *vm)
{
  while (vm_step(vm) == VM_OK);
}
