#include <avr/io.h>
#include "eeprom.h"

char EEPROM_read(char addr) {

  /* Wait until previous writing finished */
  while(EECR & _BV(EEPE));

  /* TODO: Check if this works */
  EEAR = addr;
  /* Or may (page 20)
  EEARH|= __BV(EEAR8);
  EEARL= addr;
  */

  /* EEPROM Control Register -> EEPROM Read Enable */
  EECR |= _BV(EERE);

  /* EEPROM Data Register */
  return EEDR;
}


void EEPROM_write(char addr, char byte) {

  /* Wait until previous writing finished */
  while(EECR & _BV(EEPE));

  /* EEPROM Control Register ->  EEPM1 & EEPM0 need to be zero (programming),
  EEMPE, EEPE should be zero, rest is zero anyway */
  EECR = 0x0;

  /* TODO: Check if this works */
  EEAR = addr;
  /* Or may (page 20)
  EEARH|= __BV(EEAR8);
  EEARL= addr;
  */

  /* set data EEPROM Data Register */
  EEDR = byte;

  /* EEPROM Master Program Enable */
  EECR|= _BV(EEMPE);

  /* EEPROM Program Enable */
  EECR|= _BV(EEPE);
}
