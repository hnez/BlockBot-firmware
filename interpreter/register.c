#include <avr/io.h>

#include "register.h"

uint8_t reg_get (struct vm_status_t *vm, uint8_t reg)
{
  return ((reg > 0 && reg <=3) ? vm->regs[reg] : 0);
}

uint8_t reg_set (struct vm_status_t *vm, uint8_t reg, uint8_t regalt, uint8_t val)
{
  if (reg > 0 && reg <=3) {
    // Normal target register. No redirection

    vm->regs[reg] = val;

    return (REG_OK);
  }
  else {
    // Zero target register. Redirection

    if (regalt == 0 || regalt > 3) {
      // Illegal alternative register.

      return (REG_ERR);
    }

    vm->regs[regalt] = val;

    return (REG_OK);
  }
}
