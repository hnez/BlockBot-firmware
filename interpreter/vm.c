#ifndef __UNIT_TEST__
  #include <avr/io.h>
  #include <avr/pgmspace.h>
#endif

#include "vm.h"
#include "opcodes.h"


uint8_t vm_step (struct vm_status_t *vm)
{
  uint8_t op;

  if (vm_next_op(vm, &op) == VM_OK) {
    op_cb_t cb= (op_cb_t)pgm_read_fktptr(&op_opmap[op_dec_mayor(op)]);

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
