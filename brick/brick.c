#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>


  #include <rdbuf.h>
  #include "uart.h"
  #include "descriptor.h"
#endif

int main (void)
{
  uart_init();

  sei();

  for(;;){

    /* AQ received */
    if(uart.flags.forward==1
        && uart.packet_header_rcvd[0]==0x0
          && uart.packet_header_rcvd[1]==0x1){

      if(rdbuf_len(&uart.buf)==0){

        //TODO prep_aq(&uart.buf);
      } /* else finishing transmission */
      /* When the buffer ran empty immediatelly uart.flags.forward==0,
         therefore this shouldnt lead to an unwanted second iteration */
    }
  }
  return (0);
}
