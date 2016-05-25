#pragma once

#ifndef __UNIT_TEST__
  #include <avr/io.h>
  #include <avr/pgmspace.h>
#endif

#include "vm.h"

#define MEM_OK 1
#define MEM_ERR 0

#define MEM_LEN 8

typedef uint8_t (*mem_getcb_t) (struct vm_status_t *, uint8_t, uint8_t *);
typedef uint8_t (*mem_setcb_t) (struct vm_status_t *, uint8_t, uint8_t);

struct mem_slot {
  mem_getcb_t get;
  mem_setcb_t set;
};

extern const struct mem_slot mem_map[MEM_LEN];

inline uint8_t mem_get(struct vm_status_t *vm, uint8_t addr, uint8_t *val)
{
  if (addr < MEM_LEN) {
    mem_getcb_t cb= (mem_getcb_t)pgm_read_fktptr(&mem_map[addr].get);

    if (!cb) {
      return (MEM_ERR);
    }

    return(cb(vm, addr, val));
  }
  else {
    return (MEM_ERR);
  }
}

inline uint8_t mem_set(struct vm_status_t *vm, uint8_t addr, uint8_t val)
{
  if (addr < MEM_LEN) {
    mem_setcb_t cb= (mem_setcb_t)pgm_read_fktptr(&mem_map[addr].set);

    if (!cb) {
      return (MEM_ERR);
    }

    return (cb(vm, addr, val));
  }
  else {
    return (MEM_ERR);
  }
}
