#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>

  #include <rdbuf.h>
  #include "uart.h"
#endif

  // TODO Change this
  char descriptor[10] = {"0123456789"}
  /* I used this a few times in the project. We ne to replace that aswell */

int main (void)
{
  uart_init();

  sei();

  for(;;){

    /* AQ received */
    if(uart_status.flags.forward==1
        && uart_status.packet_header_rcvd[0]==0x0
          && uart_status.packet_header_rcvd[1]==0x1){

      if(rdbuf_len(&uart_status.buf)>0){

        /* reserve space */
        rdbuf_reserve(&uart_status.buf, sizeof(descriptor));

        /* process reservation */
        for(int c=0;c<sizeof(descriptor);c++){
          if(rdbuf_reserve_push(&uart_status.buf, descriptor[c]) < 0){
            /* full, this shouldnt happen since the space got reserved */
          }
        }

      } /* else finishing transmission */
      /* When the buffer ran empty immediatelly uart_status.flags.forward==0,
         therefore this shouldnt lead to an unwanted second iteration */
    }
  }
  return (0);
}
