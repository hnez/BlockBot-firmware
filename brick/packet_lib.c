#include <avr/io.h>
#include "descriptor.h"

#include <stdbool.h>
#include <rdbuf.h>


#define EEPROM_SPACE 512 /* bytes */
#define EEPROM_HDR_LEN 4 /* Mnemonic, payload_length */

/* packets (16bit) */
#define CHAIN_AQ 0x0001

#define BRICK_CONT 0x0100
#define BRICK_NAME 0x0101
#define BRICK_BC 0x0102
#define BRICK_PREP 0x0103

#define TMTY_BRNR 0x0200
#define TMTY_BAT 0x0201

#define PGM_DATA 0x0300
#define PGM_STAT 0x0301

#define ERR_TX 0xff00
/* ---------------- */







int16_t cont_len_without_prep(uint16_t cont_index){
  /* get the length of a container minus the BRICK_PREP len */

  uint16_t cont = eeprom_read_word(cont_index);
  uint16_t len = eeprom_read_word(cont_index+2);
  uint16_t l_index = cont_index + len;


  /* permits subpackets */
  if(cont==CHAIN_AQ || cont==BRICK_CONT){

    /* init values */
    bool has_prep = true;
    int16_t prep_index, prep_len;
    int16_t prep_xedni = cont_index + EEPROM_HDR_LEN;

    while(has_prep){

      prep_index = nth_packet_by_type(BRICK_PREP, prep_xedni, l_index-prep_xendi, 1);

      if(prep_index>0){
        prep_len = eeprom_read_word((uint16_t)prep_index+2);
        len -= (prep_len + EEPROM_HDR_LEN);
        prep_xedni = prep_index + EEPROM_HDR_LEN + prep_len;
      }
      else {
        /* No BRICK_PREP left */
        return len;
      }
    }
  }
  else {
    /* No container at cont_index */
    return (-6);
  }
}



/*------------------------dependent-methods-------------------------*/



int16_t nth_subpacket_by_type(uint16_t packet, uint16_t cont_index, uint8_t n){

  return (nth_packet_by_type(cont_index+EEPROM_HDR_LEN, eeprom_read_word(cont_index+2), n, packet));
}



int16_t nth_subpacket_by_index(uint16_t cont_index, uint8_t n){

  return (nth_packet_by_index(cont_index+EEPROM_HDR_LEN, eeprom_read_word(cont_index+2), n));
}



/*------------------------independent-methods-------------------------*/



int16_t nth_packet_by_type(uint16_t packet, uint16_t f_index, uint16_t len, uint8_t n){
  /* find the nth packet of a type */

  if(n==0) return (-5);

  uint16_t packet_index, brick_word;
  uint8_t i = 0;

  for(packet_index=f_index;packet_index<f_index+len;packet_index++){
    brick_word = eeprom_read_word(packet_index);

    if(brick_word==packet){
      i++;
      if(i==n) break;
    }

    if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
        || brick_word==BRICK_NAME || brick_word==BRICK_BC
        || brick_word==BRICK_PREP || brick_word==TMTY_BRNR
        || brick_word==TMTY_BAT || brick_word==PGM_DATA
        || brick_word==PGM_STAT || brick_word==ERR_TX){
      packet_index+= eeprom_read_word(packet_index+2) + EEPROM_HDR_LEN -1;
      /* -1 because packet_index++ */
    }
  }

  if(packet_index>=EEPROM_SPACE - EEPROM_HDR_LEN){
    /* Not found */
    return (-1);
  }

  return packet_index;
}



int16_t nth_packet_by_index(uint16_t f_index, uint16_t len, uint8_t n){
  /* find the nth packet */

  if(n==0) return (-5);
  if(f_index+len>=EEPROM_SPACE){
    /* Range wider than EEPROM_SPACE */
    return (-3);
  }

  uint16_t packet_index, brick_word;
  uint8_t i = 0;

  for(packet_index=f_index;packet_index<f_index+len;packet_index++){
    brick_word = eeprom_read_word(packet_index);


    if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
        || brick_word==BRICK_NAME || brick_word==BRICK_BC
        || brick_word==BRICK_PREP || brick_word==TMTY_BRNR
        || brick_word==TMTY_BAT || brick_word==PGM_DATA
        || brick_word==PGM_STAT || brick_word==ERR_TX){

      i++;
      if(i==n) break;

      packet_index+= eeprom_read_word(packet_index+2) + EEPROM_HDR_LEN -1;
    }
  }


  /* Not found */
  if(packet_index>= f_index+len - EEPROM_HDR_LEN){
    return (-1);
  }

  return (packet_index);
}



/*----------------------------------------------------------*/
