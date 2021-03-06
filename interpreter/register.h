#pragma once

#include "vm.h"

#define REG_OK 1
#define REG_ERR 0

uint8_t reg_get (struct vm_status_t *, uint8_t);
uint8_t reg_set (struct vm_status_t *, uint8_t, uint8_t, uint8_t);
