#pragma once

#include <avr/io.h>

char EEPROM_read(char addr);
void EEPROM_write(char addr, char byte);
