#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <rdbuf.h>


  #include <rdbuf.h>
  #include "uart.h"
  #include "pkt_lib.h"
#endif

/* TODO Kann man die weglassen, weil die sind ja schon in pkt_lib */
#define BRICK_CONT 0x0100

#define EEPROM_HDR_LEN 4 /* Mnemonic, payload_length */
#define EEPROM_SPACE 512 /* bytes */

int main (void)
{
  /* #########Analyse#EEPROM######### */

  /* Get BRICK_CONT index */
  int16_t signed_brick_cont_index =
             nth_pkt_by_type(BRICK_CONT, 0, EEPROM_SPACE, 1);
  uint16_t brick_cont_index;

  /* Check if valid */
  if(signed_brick_cont_index>0){
    /* Two's complement to unsigned */
    brick_cont_index =
          ~(1 << 15) & ~((uint16_t)signed_brick_cont_index - 1);
  } else {
    /* No BRICK_CONT found */
    return (-1);
  }

  /* Get BRICK_CONT length */
  int16_t signed_brick_cont_payload_len =
              brick_cont_len_without_prep(brick_cont_index);
  uint16_t brick_cont_payload_len;

  if(signed_brick_cont_payload_len>0){
    brick_cont_payload_len =
            ~(1 << 15) & ~((uint16_t)signed_brick_cont_payload_len - 1);
  } /* else { Imposible Error } */

  /* ################################ */


  uart_init();
  sei();

  for(;;){

    /* AQ received */
    if(uart.flags.forward==1
        && uart.pkt_header_rcvd[0]==0x0
          && uart.pkt_header_rcvd[1]==0x1){

      if(rdbuf_len(&uart.buf)==0){

        make_aq();
      } /* else finishing transmission */
      /* When the buffer ran empty immediatelly uart.flags.forward==0,
         therefore this shouldnt lead to an unwanted second iteration */
    }
  }
  return (0);
}

uint8_t make_aq(){

  /* Reserve the precalculated len */
  rdbuf_reserve(&uart.buf,
            brick_cont_payload_len + EEPROM_HDR_LEN + 2); /* CHSUM */

  /* init */
  uint16_t pkt_index = brick_cont_index + EEPROM_HDR_LEN;
  uint16_t brick_word = (uint16_t)eeprom_read_byte(pkt_index);
  uint16_t resv_index = EEPROM_HDR_LEN + 2; /* CKSUM */


  /* push everything that isnt BRICK_PREP */
  for(pkt_index+= 1;
        pkt_index<brick_cont_index + EEPROM_HDR_LEN + eeprom_read_word(brick_cont_index+2);
        pkt_index++){

    shift(&brick_word, eeprom_read_byte(pkt_index));

    if(brick_word==BRICK_PREP){

      /* Skip the paket
       *     payload_length     HDR_restlenght */
      pkt_index += eeprom_read_word(pkt_index+1) + 3;

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte(pkt_index);;
    }
    else {
      /* If not BRICK_PREP, put byte in buffer */
      rdbuf_put_resv(&uart.buf, resv_index, (uint8_t)(brick_word >> 8));
      resv_index++;
    }
  }

  pkt_index = brick_cont_index + EEPROM_HDR_LEN;
  brick_word = (uint16_t)eeprom_read_byte(pkt_index);

  /* run BRICK_PREP */
  for(pkt_index+=1;
         pkt_index<brick_cont_index + EEPROM_HDR_LEN + eeprom_read_word(brick_cont_index+2);
         pkt_index++){

    shift(&brick_word, eeprom_read_byte(pkt_index));

    if(brick_word == BRICK_PREP){
      //rdbuf_put_resv(&uart.buf, pos, val);
      /* TODO Leonard */


      /* Skip the paket
       *     payload_length     HDR_restlenght */
      pkt_index += eeprom_read_word(pkt_index+1) + 3;

      /* init brick_word for next loop */
      brick_word = (uint16_t)eeprom_read_byte(pkt_index);;
    }
  }


  /* Create AQ header
   * As the length of the receiving paket must be known first
   * this happens at last */
  rdbuf_put_resv(&uart.buf, 0, 0x0); /* AQ1 */
  rdbuf_put_resv(&uart.buf, 1, 0x1); /* AQ2 */
  uint16_t aq_len = brick_cont_payload_len +
  (uint16_t)(uart.pkt_header_rcvd[2] << 8) | (uint16_t)(uart.pkt_header_rcvd[3])
  + 2; /* CKSUM */
  rdbuf_put_resv(&uart.buf, 2, (uint8_t)(8 >> aq_len));
  rdbuf_put_resv(&uart.buf, 3, (uint8_t)(aq_len&0xFF));

  rdbuf_fin_resv(&buf);
}
