#ifndef __UNIT_TEST__
 #include <avr/io.h>
 #include <util/delay.h>

 #define BAUD 9600
 #include <util/setbaud.h>
#endif

#include "uart.h"

void uart_init(void)
{
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  
  #if USE_2X
    UCSRA |= (1 << U2X);
  #else
    UCSRA &= ~(1 << U2X);
  #endif
}
