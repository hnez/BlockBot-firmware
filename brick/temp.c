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
   * to reserve the needed space in the rdbuffer. */
  uint8_t bytecode_len = 0;

  uint8_t last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);
  uint8_t cur_byte;
  for(uint16_t i = brick_cont_index + EEPROM_HDR_LEN + 1; i < brick_cont_index + EEPROM_HDR_LEN + brick_cont_len;i++){

    cur_byte = eeprom_read_byte(i);

    /* Scan for BRICK_PREP */
    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){

      i += eeprom_read_word(i+1) + 2;

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

      i += eeprom_read_word(i+1) + 2;

      last_byte = eeprom_read_byte(i);
    }
    else {
      rdbuf_put_resv(buf, i - brick_cont_index - EEPROM_HDR_LEN - 1, (char)last_byte);
      last_byte = cur_byte;
    }
  }

   /* Handle BRICK_PREP */
   last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);
   for(uint16_t i = brick_cont_index + EEPROM_HDR_LEN + 1; i< brick_cont_index + EEPROM_HDR_LEN + brick_cont_len;i++){

    cur_byte = eeprom_read_byte(i);

    /* Scan for BRICK_PREP */
    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){

      uint16_t brick_prep_len = eeprom_read_word(i+1);
      uint16_t brick_prep_para = eeprom_read_word(i+3);

      /* addresses */
      //TODO
      for(i+=5;i<brick_prep_len;i+=2){
        //rdbuf_put_resv(buf, i - brick_cont_index - EEPROM_HDR_LEN - 1, (char)last_byte);
      }
    }
   }


  rdbuf_finish_resv(buf);
  return (0);
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
