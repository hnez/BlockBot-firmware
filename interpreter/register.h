#pragma once

#include "vm.h"

#define REG_OK 1
#define REG_ERR 0

inline uint8_t reg_get (uint8_t reg)
{
  return ((reg > 0 && reg <=3) ? vm_status.regs[reg] : 0);
}

inline uint8_t reg_set (uint8_t reg, uint8_t regalt, uint8_t val)
{
  if (reg > 0 && reg <=3) {
    // Normal target register. No redirection

    vm_status.regs[reg] = val;

    return (REG_OK);
  }
  else {
    // Zero target register. Redirection

    if (regalt == 0 || regalt > 3) {
      // Illegal alternative register.

      return (REG_ERR);
    }

    vm_status.regs[regalt] = val;

    return (REG_OK);
  }
}
