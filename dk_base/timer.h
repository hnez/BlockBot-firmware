#pragma once

#include <vm.h>

void timer_init(void);
uint8_t mem_gettimer (struct vm_status_t *, uint8_t, uint8_t *);
uint8_t mem_settimer (struct vm_status_t *, uint8_t, uint8_t);
