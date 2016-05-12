#include <avr/io.h>
#include <avr/pgmspace.h>
#include "vm.h"
#include "opcodes.h"

struct vm_status_t vm_status;

uint8_t vm_step (void)
{
  if (vm_verify_jmp(VM_JMP_FWD, 1)) {
    uart_puts("Jump Okay\r\n");
    
    uint8_t op= vm_next_op();

    // "ISO C forbids conversion of object pointer to function pointer type"
    //  op_cb_t cb= (op_cb_t)pgm_read_ptr(&op_opmap[op_dec_mayor(op)]);
    // This workaround does not feel quite right:
    op_cb_t cb= (op_cb_t)pgm_read_word(&op_opmap[op_dec_mayor(op)]);
    uart_puthb(op_dec_mayor(op));
    uart_puthb((uint16_t)cb >> 8);
    uart_puthb((uint16_t)cb&0xff);

    return (cb(op));
  }
  else {
    uart_puts("Jump Broken\r\n");
    
    return (VM_ERR);
  }
}

void vm_run (void)
{
  while (vm_step() == VM_OK);
}
