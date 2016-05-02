#pragma once

#define MEM_OK 1
#define MEM_ERR 0

#define MEM_LEN 8

typedef uint8_t (*mem_getcb_t) (uint8_t);
typedef uint8_t (*mem_setcb_t) (uint8_t, uint8_t);

extern PROGMEM const mem_getcb_t mem_getmap[MEM_LEN];
extern PROGMEM const mem_setcb_t mem_setmap[MEM_LEN];

inline uint8_t mem_get(uint8_t addr)
{
  if (addr < MEM_LEN) {
    mem_getcb_t cb= (mem_getcb_t)pgm_read_ptr(&mem_getmap[addr]);

    return (cb(addr));
  }
  else {
    return (MEM_ERR);
  }
}

inline uint8_t mem_set(uint8_t addr, uint8_t val)
{
  if (addr < MEM_LEN) {
    mem_setcb_t cb= (mem_getcb_t)pgm_read_ptr(&mem_setmap[addr]);

    return (cb(addr, val));
  }
  else {
    return (MEM_ERR);
  }
}
