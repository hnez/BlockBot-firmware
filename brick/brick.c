#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <rdbuf.h>


  #include <rdbuf.h>
  #include "uart.h"
  #include "packet_lib.h"
#endif

#define BRICK_CONT 0x0100
#define EEPROM_HDR_LEN 4 /* Mnemonic, payload_length */

#define EEPROM_SPACE 512 /* bytes */

int main (void)
{
  uart_init();
  /* Get BRICK_CONT index */
  int16_t signed_brick_cont_index = nth_packet_by_type(BRICK_CONT, 0, EEPROM_SPACE, 1)
  uint16_t brick_cont_index;

  if(signed_brick_cont_index>0){
    /* Two's complement to unsigned */
    brick_cont_index = ~(1 << 15) & ~((uint16_t)signed_brick_cont_index - 1);
  } else {
    /* No BRICK_CONT found */
    return (-1);
  }

  /* Get BRICK_CONT length */
  int16_t temp_brick_cont_payload_len = cont_len_without_prep(brick_cont_index);
  uint16_t brick_cont_payload_len;

  if(temp_brick_cont_payload_len>0){
    brick_cont_payload_len = ~(1 << 15) & ~((uint16_t)temp_brick_cont_payload_len - 1);
  } else {
    /* Error */
    return (-1);
  }
  sei();

  for(;;){

    /* AQ received */
    if(uart.flags.forward==1
        && uart.packet_header_rcvd[0]==0x0
          && uart.packet_header_rcvd[1]==0x1){

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
  rdbuf_reserve (&uart.buf, brick_cont_payload_len);

  /* push everything that isnt BRICK_PREP */
  uint8_t last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);
  uint8_t cur_byte;

  for(uint16_t i =brick_cont_index + EEPROM_HDR_LEN + 1;i<brick_cont_index + EEPROM_HDR_LEN + eeprom_read_word(brick_cont_index+2);i++){
    cur_byte = eeprom_read_byte(i);

    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){

      /* Skip the paket
       *     payload_length     HDR_restlenght */
      i += eeprom_read_word(i+1) + 2;

      /* init next loop */
      last_byte = eeprom_read_byte(i);
    }
    else {
      /* If not BRICK_PREP, put byte in buffer
       *                       current position minus offset */
      rdbuf_put_resv(&uart.buf, i-(brick_cont_index + EEPROM_HDR_LEN + 1), last_byte)
      last_byte = cur_byte;
    }
  }

  last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);

  /* run BRICK_PREP */
  for(uint16_t i =brick_cont_index + EEPROM_HDR_LEN + 1;i<brick_cont_index + EEPROM_HDR_LEN + eeprom_read_word(brick_cont_index+2);i++){
    cur_byte = eeprom_read_byte(i);

    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){
      //rdbuf_put_resv(&uart.buf, pos, val);
      /* TODO Leonard */


      /* init next loop */
      i += eeprom_read_word(i+1) + 2;
      last_byte = eeprom_read_byte(i);
    }
    else {
      last_byte = cur_byte;
    }
  }


  /* Create AQ header
   * As the length of the receiving paket must be known first
   * this happens at last */
  rdbuf_put_resv(&uart.buf, 0, 0x0);
  rdbuf_put_resv(&uart.buf, 1, 0x1);
  uint16_t aq_len = brick_cont_payload_len +
  (uint16_t)(uart.packet_header_rcvd[2] << 8) | (uint16_t)(uart.packet_header_rcvd[3]);

  rdbuf_finish_resv(&buf);
}
