#pragma once

#include <vm.h>

uint8_t mem_getram (struct vm_status_t *, uint8_t, uint8_t *);
uint8_t mem_setram (struct vm_status_t *, uint8_t, uint8_t);
