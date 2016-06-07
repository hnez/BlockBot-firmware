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
  int16_t resv_len = cont_len_without_prep((uint16_t)nth_packet_by_type(BRICK_CONT, 0, EEPROM_SPACE, 1));
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
  rdbuf_reserve (&uart.buf, resv_len);

  /* find brick container */
  int16_t brick_cont_index = nth_packet_by_type(BRICK_CONT, 0, EEPROM_SPACE, 1);
  uint8_t last_byte = eeprom_read_byte(brick_cont_index + EEPROM_HDR_LEN);
  uint8_t cur_byte;
  /* push everything that isnt BRICK_PREP */
  for(uint16_t i =brick_cont_index + EEPROM_HDR_LEN + 1;i<brick_cont_index + EEPROM_HDR_LEN + eeprom_read_word(brick_cont_index+2);i++){
    cur_byte = eeprom_read_byte(i);

    if((uint16_t)(last_byte << 8) | (uint16_t)(cur_byte) == BRICK_PREP){
      i += eeprom_read_word(i+1) + 2;

      last_byte = eeprom_read_byte(i);
    }
    else {
      rdbuf_put_resv(&uart.buf, i-(brick_cont_index + EEPROM_HDR_LEN + 1), last_byte)
      last_byte = cur_byte;
    }

  }

  /* do BRICK_PREP */
}
