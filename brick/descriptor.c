#include <avr/io.h>
#include "descriptor.h"

#include <stdbool.h>
#include <rdbuf.h>
#include <avr/eeprom.h>

#define EEPROM_SPACE 512 /* bytes */
#define EEPROM_HDR_LEN 4 /* Mnemonic, payload_length */
#define BRICK_CONT 0x0100 /* (16bit) */



uint8_t prep_AQ(struct rdbuf_t *buf)
{

  /* find brick container */
  uint16_t brick_cont_index = 0;
  bool found_container = false;

  while(!found_container){
    if(eeprom_read_word(brick_cont_index)==BRICK_CONT){ // reads 16bit, returns uint16_t
      found_container = true;
    }
    else {
      brick_cont_index += EEPROM_HDR_LEN + eeprom_read_word(brick_cont_index+2);

      /* The Attiny85 contains 512bytes of EEPROM */
      if(brick_cont_index>EEPROM_SPACE -5){
        /* BRICK_CONT not found */
        return (-1);
      }
    }
  }


  uint16_t brick_cont_len = eeprom_read_word(brick_cont_index+2);
  /* First we need to find out how long the brick descriptor bytecode is
   * to reserve the needed space in the rdbuffer.
   * Because eeprom operations are slow, we dont read the payload byte by byte */
  uint8_t bytecode_len = 0;

  uint8_t last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);
  uint8_t cur_byte;
  for(uint16_t i = brick_cont_index + EEPROM_HDR_LEN + 1; i < brick_cont_index + EEPROM_HDR_LEN + brick_cont_len;i++){

    cur_byte = eeprom_read_byte(i);

    /* Scan for BRICK_PREP */
    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){
      /* Because the EEPROM ist 512bytes long, and this is a subpacket, I assume
       * that it wont be longer than 255 bytes */
      i += eeprom_read_byte(i+2) + 2;

      last_byte = eeprom_read_byte(i);
    }
    else {
     bytecode_len++;
     last_byte = cur_byte;
    }
  }
  /* Fix because the last byte was skipped */
  if(bytecode_len>0) bytecode_len++;

  rdbuf_reserve(buf, bytecode_len);


  /* Everything in payload, that isnt BRICK_PREP, is brick descriptor bytecode
   * and belongs in the roundbuffer */
  last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);
  for(uint16_t i = brick_cont_index + EEPROM_HDR_LEN + 1; i< brick_cont_index + EEPROM_HDR_LEN + brick_cont_len;i++){

    cur_byte = eeprom_read_byte(i);

    /* Scan for BRICK_PREP */
    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){
      /* Because the EEPROM ist 512bytes long, and this is a subpacket, I assume
       * that it wont be longer than 255 bytes */
      i += eeprom_read_byte(i+2) + 2;

      last_byte = eeprom_read_byte(i);
    }
    else {
      rdbuf_put_resv(buf, i - brick_cont_index - EEPROM_HDR_LEN - 1, (char)last_byte)
      last_byte = cur_byte;
    }
  }

   /* Handle BRICK_PREP */
  //TODO


  rdbuf_finish_resv(buf);
  return (0);
}
