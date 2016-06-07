#pragma once

#include <avr/io.h>

#include <stdbool.h>
#include <avr/eeprom.h>

int16_t cont_len_without_prep(uint16_t cont_index);
int16_t nth_subpacket_by_type(uint16_t packet, uint16_t cont_index, uint8_t n);
int16_t nth_subpacket_by_index(uint16_t cont_index, uint8_t n);
int16_t nth_packet_by_type(uint16_t packet, uint16_t f_index, uint16_t len, uint8_t n);
int16_t nth_packet_by_index(uint16_t f_index, uint16_t len, uint8_t n);
