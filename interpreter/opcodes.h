#pragma once

#ifndef __UNIT_TEST__
  #include <avr/io.h>
#endif

#include "vm.h"

#define OP_OK 1
#define OP_ERR 0

#define op_dec_mayor(o) ((o & 0xf0) >> 4)

typedef uint8_t (*op_cb_t)(struct vm_status_t*, uint8_t);

extern const op_cb_t op_opmap[16];
