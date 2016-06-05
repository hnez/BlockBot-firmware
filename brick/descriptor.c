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



/*------------------------high-methods-------------------------*/



int16_t packet_len(uint16_t packet_index){
  /* get the length of a packet minus the subpacketlength */

  uint16_t cont = eeprom_read_word(packet_index);
  uint16_t len = eeprom_read_word(packet_index+2);


  /* permits subpackets */
  if(cont==CHAIN_AQ || cont==BRICK_CONT){

    bool has_subpackets = true;
    uint8_t n = 1;

    while(has_subpackets){

      if(nth_subpacket(packet_index, n)){

      }
    }

  }
  /* prohibits subpackets */
  else if(
      || cont==BRICK_NAME || cont==BRICK_BC
      || cont==BRICK_PREP || cont==TMTY_BRNR
      || cont==TMTY_BAT || cont==PGM_DATA
      || cont==PGM_STAT || cont==ERR_TX){

    return (len);
  }
  else {
    /* No packet at packet_index */
    return (-2);
  }
}




int16_t subpacket(uint16_t cont_index){
  /* find the first subpacket of a container */
  return (packet_index_range(cont_index+EEPROM_HDR_LEN, eeprom_read_word(cont_index+2)));
}
int16_t nth_subpacket(uint16_t cont_index, uint8_t n){
  /*  find the nth subpacket of a container */
  return (nth_packet_index_range(cont_index+EEPROM_HDR_LEN, eeprom_read_word(cont_index+2), n))
  /*
  int16_t old_nth_subpacket(uint16_t cont_index, uint8_t n){

    int16_t ith_index;
    uint16_t ith_xedni = cont_index + EEPROM_HDR_LEN;

    for(uint8_t i=0;i<n;i++){
      ith_index = packet_index_range(ith_xedni,eeprom_read_word(cont_index+2));

      if(ith_index<0){

        return (ith_index);
      }

      ith_xedni += eeprom_read_word(ith_index+2);
    }

    return ith_index;
  }
  */
}

int16_t subpacket(uint16_t cont_index, uint16_t subpacket){
  /* find the first subpacket of a container of a type */
  return(packet_index())
}






















/*------------------------low-methods-------------------------*/


int16_t packet_index(uint16_t packet){
  /* find the first packet of a type */


  uint16_t packet_index, brick_word;

  for(packet_index=0;packet_index<EEPROM_SPACE;packet_index++){
    brick_word = eeprom_read_word(packet_index);

    if(brick_word==packet){
      break;
    }

    /* Other packet */
    else if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
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


int16_t nth_packet_index(uint16_t packet uint8_t n){
  /* find the nth packet of a type */

  if(n==0) return (-5);

  uint16_t packet_index, brick_word;
  uint8_t i = 0;

  for(packet_index=0;packet_index<EEPROM_SPACE;packet_index++){
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


int16_t packet_index_range(uint16_t f_index, uint16_t len){
  /* find the first packet in a range */

  if(f_index+len>=EEPROM_SPACE){
    /* Range wider than EEPROM_SPACE */
    return (-3);
  }

  uint16_t packet_index, brick_word;


  for(packet_index=f_index;packet_index<f_index+len;packet_index++){
    brick_word = eeprom_read_word(packet_index);


    if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
        || brick_word==BRICK_NAME || brick_word==BRICK_BC
        || brick_word==BRICK_PREP || brick_word==TMTY_BRNR
        || brick_word==TMTY_BAT || brick_word==PGM_DATA
        || brick_word==PGM_STAT || brick_word==ERR_TX){
      return packet_index;
    }
  }


  /* Not found */
  return (-1);
}


int16_t nth_packet_index_range(uint16_t f_index, uint16_t len, uint8_t n){
  /* find the nth packet in a range */

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
