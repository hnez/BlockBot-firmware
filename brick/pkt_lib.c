//###########################################################
//#TODO Replace eeprom_read_word with 2x eeprom_read_byte   #
//###########################################################

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











/*------------------------dependent-methods-------------------------*/

int16_t brick_cont_len_without_prep(uint16_t cont_index){
  /* get the length of a container minus the BRICK_PREP len */

  uint16_t brick_cont = eeprom_read_word(cont_index);
  uint16_t brick_cont_len = eeprom_read_word(cont_index+2);
  uint16_t l_index = cont_index + len;


  if(brick_cont==BRICK_CONT){

    /* init values */
    bool has_prep = true;
    int16_t signed_prep_index
    uint16_t prep_index, prep_len;
    unt16_t prep_xedni = cont_index + EEPROM_HDR_LEN;

    while(has_prep){

      signed_prep_index = nth_pkt_by_type(BRICK_PREP, prep_xedni, l_index-prep_xendi, 1);

      if(signed_prep_index>0){
        prep_index = ~(1 << 15) & ~((uint16_t)signed_prep_index - 1);
        prep_len = eeprom_read_word(prep_index+2);
        brick_cont_len -= (prep_len + EEPROM_HDR_LEN);
        prep_xedni = prep_index + EEPROM_HDR_LEN + prep_len;
      }
      else {
        /* No BRICK_PREP left */
        return len;
      }
    }
  }
  else {
    /* No BRICK_CONT at cont_index */
    return (-6);
  }
}



int16_t nth_subpkt_by_type(uint16_t pkt, uint16_t cont_index, uint8_t n){

  return (nth_pkt_by_type(pkt, cont_index+EEPROM_HDR_LEN, eeprom_read_word(cont_index+2), n));
}



int16_t nth_subpkt_by_index(uint16_t cont_index, uint8_t n){

  return (nth_pkt_by_index(cont_index+EEPROM_HDR_LEN, eeprom_read_word(cont_index+2), n));
}



/*------------------------independent-methods-------------------------*/
void shift(uint16_t *word, uint8_t byte){
  *word = (word << 8) | (uint16_t)byte;
}


int16_t nth_pkt_by_type(uint16_t pkt, uint16_t f_index, uint16_t len, uint8_t n){
  /* find the nth packet of a type */

  if(n==0) return (-5);
  if(f_index+len>=EEPROM_SPACE){
    /* Range wider than EEPROM_SPACE */
    return (-3);
  }

  uint16_t pkt_index = f_index;
  uint16_t brick_word = (uint16_t)eeprom_read_byte(pkt_index);
  uint8_t i = 0; /* counts from 0 to n*/

  for(pkt_index+= 1;pkt_index<f_index+len;pkt_index++){
    shift(&brick_word, eeprom_read_byte(pkt_index));

    /* If nth packet found, return */
    if(brick_word==pkt){
      i++;
      if(i==n) break;
    }

    /* If packet found, skip */
    if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
        || brick_word==BRICK_NAME || brick_word==BRICK_BC
        || brick_word==BRICK_PREP || brick_word==TMTY_BRNR
        || brick_word==TMTY_BAT || brick_word==PGM_DATA
        || brick_word==PGM_STAT || brick_word==ERR_TX){

      /* Jump behind packet
       * packet_index currently on HRD_byte 2 */
      pkt_index+= eeprom_read_word(pkt_index+1) + 3;
      /* Now right behind packet */

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte(pkt_index);
    }
  }

  if(pkt_index>=f_index+len){
    /* Not found */
    return (-1);
  }

  return (pkt_index - 1);
}



int16_t nth_pkt_by_index(uint16_t f_index, uint16_t len, uint8_t n){
  /* find the nth packet */

  if(n==0) return (-5);
  if(f_index+len>=EEPROM_SPACE){
    /* Range wider than EEPROM_SPACE */
    return (-3);
  }

  uint16_t pkt_index = f_index;
  uint16_t brick_word = eeprom_read_word(pkt_index);
  uint8_t i = 0; /* counts from 0 to n*/

  for(pkt_index+= 1;pkt_index<f_index+len;pkt_index++){
    shift(&brick_word, eeprom_read_byte(pkt_index));

    // If packet found, skip */
    if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
        || brick_word==BRICK_NAME || brick_word==BRICK_BC
        || brick_word==BRICK_PREP || brick_word==TMTY_BRNR
        || brick_word==TMTY_BAT || brick_word==PGM_DATA
        || brick_word==PGM_STAT || brick_word==ERR_TX){

      /* if nth packet, return */
      i++;
      if(i==n) break;

      pkt_index+= eeprom_read_word(pkt_index+1) + 3;

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte(pkt_index);
    }
  }


  /* Not found */
  if(pkt_index >= f_index+len){
    return (-1);
  }

  return (pkt_index - 1);
}



/*----------------------------------------------------------*/
