#pragma once

#include <avr/io.h>

#include <stdbool.h>
#include <rdbuf.h>
#include <avr/eeprom.h>

uint8_t prep_aq(struct rdbuf_t *buf);
