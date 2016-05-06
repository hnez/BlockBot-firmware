#pragma once

#include "vm.h"

#define REG_OK 1
#define REG_ERR 0

uint8_t reg_get (uint8_t);
uint8_t reg_set (uint8_t, uint8_t, uint8_t);
