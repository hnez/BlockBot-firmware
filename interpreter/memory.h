#include <avr/io.h>
#include <avr/pgmspace.h>

#pragma once

#define MEM_OK 1
#define MEM_ERR 0

#define MEM_LEN 8

typedef uint8_t (*mem_getcb_t) (uint8_t);
typedef uint8_t (*mem_setcb_t) (uint8_t, uint8_t);

struct mem_slot {
  mem_getcb_t get;
  mem_setcb_t set;
};

extern const struct mem_slot mem_map[MEM_LEN];

inline uint8_t mem_get(uint8_t addr)
{
  if (addr < MEM_LEN) {

    // "ISO C forbids conversion of object pointer to function pointer type"
    //  mem_getcb_t cb= (mem_getcb_t)pgm_read_ptr(&mem_getmap[addr]);
    // This workaround does not feel quite right:
    mem_getcb_t cb= (mem_getcb_t)pgm_read_word(&mem_map[addr].get);

    if (!cb) {
      return (MEM_ERR);
    }

    return (cb(addr));
  }
  else {
    return (MEM_ERR);
  }
}

inline uint8_t mem_set(uint8_t addr, uint8_t val)
{
  if (addr < MEM_LEN) {
    // "ISO C forbids conversion of object pointer to function pointer type"
    //   mem_setcb_t cb= (mem_getcb_t)pgm_read_ptr(&mem_setmap[addr]);
    // This workaround does not feel quite right:
    mem_setcb_t cb= (mem_setcb_t)pgm_read_word(&mem_map[addr].set);

    if (!cb) {
      return (MEM_ERR);
    }
    
    return (cb(addr, val));
  }
  else {
    return (MEM_ERR);
  }
}
