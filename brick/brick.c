#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <avr/eeprom.h>


  #include <rdbuf.h>
  #include <pktpsr.h>

  #include "uart.h"
#endif


struct {
  uint16_t index;
  uint16_t len; /* without BRICK_PREP */
  uint16_t total_len;
} brick_cont;



uint8_t init_brick_cont(){

  /* Get BRICK_CONT index */
  int16_t signed_brick_cont_index =
             nth_pkt_by_type(BRICK_CONT, 0, EEPROM_SPACE, 1);
  /* Is probably 0 */

  /* Check if valid */
  if(signed_brick_cont_index>=0){
    brick_cont.index = (uint16_t)signed_brick_cont_index;
  } else {
    return (-1); /* No BRICK_CONT found */
  }

  brick_cont.total_len = (uint16_t)(eeprom_read_byte((uint8_t *)brick_cont.index+2) << 8)
                          | (uint16_t)(eeprom_read_byte((uint8_t *)brick_cont.index+3));

  /* Get BRICK_CONT length */
  int16_t signed_brick_cont_payload_len =
              brick_cont_len_without_prep(brick_cont.index);

  /* if(signed_brick_cont_payload_len>0){ */
    brick_cont.len =
            (uint16_t)signed_brick_cont_payload_len;
  /* }  else { Impossible Error } */

  return (0);
}

uint8_t make_aq()
{

  /* Reserve the precalculated len */
  rdbuf_reserve(&uart.buf,
            AQ_HDR_LEN + EEPROM_HDR_LEN + brick_cont.len);
        /*                BRICK_CONT_HDR                   */


  /* init */
  uint16_t pkt_index = brick_cont.index + EEPROM_HDR_LEN;
  uint16_t brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);
  uint16_t resv_index = AQ_HDR_LEN + EEPROM_HDR_LEN;


  /* push everything that isnt BRICK_PREP */
  for(pkt_index+= 1;
        pkt_index < brick_cont.index + EEPROM_HDR_LEN +  brick_cont.total_len;
        pkt_index++){

    brick_word = (brick_word << 8) | (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);

    if(brick_word==BRICK_PREP){

      /* Skip the paket
       *     payload_length     HDR_restlenght */
      pkt_index += (uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+1) << 8) +
                     (uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+2)) + 3;

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);;
    }
    else {
      /* If not BRICK_PREP, put byte in buffer */
      rdbuf_put_resv(&uart.buf, resv_index, (uint8_t)(brick_word >> 8));
      resv_index++;
    }
  }


  /* init */
  pkt_index = brick_cont.index + EEPROM_HDR_LEN;
  brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);

  /* run BRICK_PREP */
  for(pkt_index+=1;
         pkt_index < brick_cont.index + EEPROM_HDR_LEN + brick_cont.total_len;
         pkt_index++){

    brick_word = (brick_word << 8) | (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);

    if(brick_word == BRICK_PREP){
      //rdbuf_put_resv(&uart.buf, pos, val);
      /* TODO */


      /* Skip the paket
       *     payload_length     HDR_restlenght */
       pkt_index += (uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+1) << 8) +
                      (uint16_t)(eeprom_read_byte((uint8_t *)pkt_index+2)) + 3;

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte((uint8_t *)pkt_index);;
    }
  }


  /* Create AQ header
   * As the length of the receiving paket must be known first
   * this happens at last */
  rdbuf_put_resv(&uart.buf, 0, 0x0); /* AQ1 */
  rdbuf_put_resv(&uart.buf, 1, 0x1); /* AQ2 */
  uint16_t aq_len = brick_cont.len +
  ((uint16_t)(uart.hdr_rvcd[2] << 8) | (uint16_t)(uart.hdr_rvcd[3])); /* CKSUM in hdr_rvcd */
  rdbuf_put_resv(&uart.buf, 2, (uint8_t)(8 >> aq_len));
  rdbuf_put_resv(&uart.buf, 3, (uint8_t)(aq_len&0xFF));

  /* CKSUM Placeholder */
  rdbuf_put_resv(&uart.buf, 4, 0x0);
  rdbuf_put_resv(&uart.buf, 5, 0x0);

  rdbuf_fin_resv(&uart.buf);

  return (0);
}



int main (void)
{

  init_brick_cont();
  uart_init();
  communicate();
  sei();

  for(;;){

    /* AQ received */
    if(!uart.flags.aq_complete
        && uart.hdr_rvcd[0]==0x0
          && uart.hdr_rvcd[1]==0x1){

      if(rdbuf_len(&uart.buf)==0){

        make_aq();
      } /* else finishing transmission */
      /* When the buffer ran empty uart.flags.rcving_header==0,
         therefore this shouldnt lead to an unwanted second iteration */
    }
    /* first brick, start aq */
    if(uart.flags.first_brick
       && !uart.flags.aq_complete) {
      if(rdbuf_len(&uart.buf)==0){

        make_aq();
      } /* else finishing transmission */
    }

    if (uart.flags.aq_complete) {
      return (0);
    }
  }
  return (0);
}
