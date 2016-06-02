#include <vm.h>


uint8_t vm_ram[3];

uint8_t mem_getram (__attribute__((unused)) struct vm_status_t *vm,
                    uint8_t addr,
                    uint8_t *val)
{
  if((addr<=7) && (addr>=5)) {
    *val= vm_ram[addr-5];
    return (MEM_OK);
  }
  else {
     return (MEM_ERR);
  }
}

uint8_t mem_setram (__attribute__((unused)) struct vm_status_t *vm,
                    uint8_t addr,
                    uint8_t val)
{
  if((addr<=7) && (addr>=5)) {
    vm_ram[addr-5]=val;
    return (MEM_OK);
  }
  else {
     return (MEM_ERR);
  }
}
