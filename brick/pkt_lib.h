#pragma once

#include <avr/io.h>

#include <stdbool.h>
#include <avr/eeprom.h>

int16_t cont_len_without_prep(uint16_t cont_index);
int16_t nth_subpkt_by_type(uint16_t pkt, uint16_t cont_index, uint8_t n);
int16_t nth_subpkt_by_index(uint16_t cont_index, uint8_t n);
int16_t nth_pkt_by_type(uint16_t pkt, uint16_t f_index, uint16_t len, uint8_t n);
int16_t nth_pkt_by_index(uint16_t f_index, uint16_t len, uint8_t n);
