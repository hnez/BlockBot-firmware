#ifndef __UNIT_TEST__
  #include <avr/io.h>
  #include <avr/eeprom.h>
#endif

#include "pktpsr.h"
#include <stdbool.h>






/*------------------------independent-methods-------------------------*/


int16_t nth_pkt_by_type(uint16_t pkt, uint16_t f_index, uint16_t len, uint8_t n)
{
  /* find the nth packet of a type */

  if(n==0) return (-5);
  if(f_index+len>EEPROM_SPACE){
    /* Range wider than EEPROM_SPACE */
    return (-3);
  }

  uint16_t pkt_index = f_index;
  uint16_t brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);
  uint8_t i = 0; /* counts from 0 to n*/

  for(pkt_index+= 1;pkt_index<f_index+len;pkt_index++){
    brick_word = (brick_word << 8) | (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);

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
      pkt_index+= ((uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+1) << 8) | (uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+2))) + 3;
      /* Now right behind packet */

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);
    }
  }

  if(pkt_index>=f_index+len){
    /* Not found */
    return (-1);
  }

  return (pkt_index - 1);
}



int16_t nth_pkt_by_index(uint16_t f_index, uint16_t len, uint8_t n)
{
  /* find the nth packet */

  if(n==0) return (-5);
  if(f_index+len>EEPROM_SPACE){
    /* Range wider than EEPROM_SPACE */
    return (-3);
  }

  uint16_t pkt_index = f_index;
  uint16_t brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);

  uint8_t i = 0; /* counts from 0 to n*/

  for(pkt_index+= 1;pkt_index<f_index+len;pkt_index++){
    brick_word = (brick_word << 8) | (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);

    //printf ("%" PRId16, brick_word);
    //printf ("\n");

    // If packet found, skip */
    if(brick_word==CHAIN_AQ || brick_word==BRICK_CONT
        || brick_word==BRICK_NAME || brick_word==BRICK_BC
        || brick_word==BRICK_PREP || brick_word==TMTY_BRNR
        || brick_word==TMTY_BAT || brick_word==PGM_DATA
        || brick_word==PGM_STAT || brick_word==ERR_TX){

      /* if nth packet, return */
      i++;
      if(i==n) break;

      pkt_index+= ((uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+1)) | (uint16_t)eeprom_read_byte((uint8_t *)pkt_index+2)) + 3;

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);
    }
  }


  /* Not found */
  if(pkt_index >= f_index+len){
    return (-1);
  }

  return (pkt_index - 1);
}




/*------------------------dependent-methods-------------------------*/

int16_t brick_cont_len_without_prep(uint16_t cont_index)
{
  /* get the length of a BRICK_CONT minus the BRICK_PREP len */

  uint16_t brick_cont = (uint16_t)(eeprom_read_byte((uint8_t *)cont_index) << 8) | (uint16_t)eeprom_read_byte((uint8_t *)cont_index+1);
  if (brick_cont!=BRICK_CONT) return(-6);

  uint16_t brick_cont_len = (uint16_t)(eeprom_read_byte((uint8_t *)cont_index+2) << 8) | (uint16_t)eeprom_read_byte((uint8_t *)cont_index+3);

  uint16_t l_index = cont_index + EEPROM_HDR_LEN + brick_cont_len;


  /* init values */
  bool has_prep = true;
  int16_t signed_prep_index;
  uint16_t prep_index, prep_len;
  uint16_t prep_xedni = cont_index + EEPROM_HDR_LEN;

  while(has_prep){

    signed_prep_index = nth_pkt_by_type(BRICK_PREP, prep_xedni, l_index-prep_xedni, 1);


    if(signed_prep_index>0){
      prep_index = (uint16_t)signed_prep_index;
      prep_len = (uint16_t)(eeprom_read_byte((uint8_t *)prep_index+2) << 8) | (uint16_t)eeprom_read_byte((uint8_t *)prep_index+3);
      brick_cont_len -= (prep_len + EEPROM_HDR_LEN);

      prep_xedni = prep_index + EEPROM_HDR_LEN + prep_len;
    }
    else {
      /* No BRICK_PREP left */
      has_prep = false;
    }
  }
  return brick_cont_len;
}


int16_t nth_subpkt_by_type(uint16_t pkt, uint16_t cont_index, uint8_t n)
{
  return (nth_pkt_by_type(pkt, cont_index+EEPROM_HDR_LEN,
    (uint16_t)(eeprom_read_byte((uint8_t *)cont_index+2) << 8) | (uint16_t)eeprom_read_byte((uint8_t *)cont_index+3), n));
}

int16_t nth_subpkt_by_index(uint16_t cont_index, uint8_t n)
{
  return (nth_pkt_by_index(cont_index+EEPROM_HDR_LEN,
            (uint16_t)(eeprom_read_byte((uint8_t *)cont_index+2) << 8) | (uint16_t)eeprom_read_byte((uint8_t *)cont_index+3), n));
}

/*----------------------------------------------------------*/
