#include "memory.h"
#include "vm.h"

PROGMEM const mem_getcb_t mem_getmap[8]= {
  mem_getmot,
  mem_getmot,
  mem_getdin,
  mem_getdout,
  mem_gettimer,
  mem_getram,
  mem_getram,
  mem_getram
};

PROGMEM const mem_setcb_t mem_setmap[8]= {
  mem_setmot,
  mem_setmot,
  mem_setdin,
  mem_setdout,
  mem_settimer,
  mem_setram,
  mem_setram,
  mem_setram
};

uint8_t mem_getmot (uint8_t addr)
{
  // TODO
  return (0xaa);
}

uint8_t mem_getdin (uint8_t addr)
{
  // TODO
  return (0xaa);
}

uint8_t mem_getdout (uint8_t addr)
{
  // TODO
  return (0xaa);
}

uint8_t mem_gettimer (uint8_t addr)
{
  // TODO
  return (0xaa);
}

uint8_t mem_getram (uint8_t addr)
{
  return ((addr >= 5 && addr < 8) ? vm_status.mem[addr-5] : 0);
}

uint8_t mem_setmot (uint8_t addr, uint8_t value)
{
  // TODO
}

uint8_t mem_setdin (uint8_t addr, uint8_t value)
{
  // TODO
}

uint8_t mem_setdout (uint8_t addr, uint8_t value)
{
  // TODO
}

uint8_t mem_settimer (uint8_t addr, uint8_t value)
{
  // TODO
}

uint8_t mem_setram (uint8_t addr, uint8_t value)
{
  if (addr >= 5 && addr < 8) {
    vm_status.mem[addr-5] = value;

    return (MEM_OK);
  }
  else {
    return (MEM_ERR);
  }
}
