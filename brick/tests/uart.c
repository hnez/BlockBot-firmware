#include <stdint.h>
#include <stdio.h>
#include <minunit.h>

int tests_run = 0;

// struct rdbuf_t {
//   char last_byte;
//   int should_fail;
//   int len;
// };

struct {
  uint8_t ddrb;
  uint8_t portb;
  uint8_t pinb;
  uint8_t ocr0a;
  uint8_t timsk;
  uint8_t tcnt0;
  uint8_t tccr0b;
  uint8_t gimsk;
  uint8_t pcmsk;
}globals;


#define DDRB   (globals.ddrb)
#define PORTB  (globals.portb)
#define PINB   (globals.pinb)
#define OCR0A  (globals.ocr0a)
#define TIMSK  (globals.timsk)
#define TCNT0  (globals.tcnt0)
#define TCCR0B (globals.tccr0b)
#define GIMSK  (globals.gimsk)
#define PCMSK  (globals.pcmsk)

#define F_CPU 8000000L

#define PB4     4
#define PB3     3
#define CS01    1
#define CS00    0
#define OCIE0A  4
#define PCIE    5

#define _BV(v) (1<<v)

#define ISR(vect) void vect (void)

#define PROGMEM

#define pgm_read_byte(ptr) (*((uint8_t *)ptr))


#define __UNIT_TEST__
#define main main_orig

#include "../uart.c"

#include <stdlib.h>
#include <strings.h>
#include <string.h> //??

#undef main

void reset_globals()
{
  bzero(&globals, sizeof(globals));
  bzero(&uart, sizeof(uart));
}

static char *test_bittimes()
{
  int lt=0;

  for (size_t i=0;i<sizeof(uart_times); i++) {
    mu_assert("Difference between bit times is very small",
              uart_times[i]-lt >= 3);

    lt= uart_times[i];
  }

  mu_assert("Bit times do not fit into 8 bit counter",
            UART_BITTIME(9) < 255);

  return 0;
}

static char *test_init()
{
  reset_globals();
  uart_init();

  mu_assert("Error TX not driven low",
            !(PORTB & _BV(PB4)) && (DDRB & _BV(PB4)));

  mu_assert("Pin change interrupt not enabled on RX pin",
            PCMSK & _BV(PB3) && GIMSK & _BV(PCIE));


  return 0;
}

static char *test_pcint_nostartbit()
{
  reset_globals();
  uart_init();

  // Set RX pin to high (no start bit)
  PINB |= _BV(PB3);

  // Backup global state
  void *oldglobals= malloc(sizeof(globals));
  void *olduartstatus= malloc(sizeof(uart));
  memcpy(oldglobals, &globals, sizeof(globals));
  memcpy(olduartstatus, &uart, sizeof(uart));

  // Execute interrupt handler
  PCINT0_vect();

  mu_assert("PCINT changed registers but there was no start bit",
            !memcmp(oldglobals, &globals, sizeof(globals)));

  mu_assert("PCINT changed uart but there was no start bit",
            !memcmp(olduartstatus, &uart, sizeof(uart)));

  return 0;
}

static char *test_pcint_startbit()
{
  reset_globals();
  uart_init();

  // Set RX pin to low (start bit)
  PINB &= ~_BV(PB3);

  PCINT0_vect();

  mu_assert("Timer wakeup time was not set correctly",
            OCR0A != 0 && TCNT0==0 && (TCCR0B & 0x07) != 0);

  mu_assert("Timer interrupt was not enabled",
            TIMSK & _BV(OCIE0A));

  mu_assert("Bit number was not set up",
            uart.bitnum == 0);

  return 0;
}

static char *test_receive()
{
  reset_globals();
  uart_init();

  // A valid aquistion request (execpt of the payload)
  char msg[]= "\x00\x01\x00\x04\xff\x6fHI";

  for (size_t i=0; i<sizeof(msg)-1; i++) {
    uint16_t symbol= _BV(9) | msg[i] << 1;

    // Set RX pin to low (start bit)
    PINB &= ~_BV(PB3);

    PCINT0_vect();

    for (int j=0;j<=9;j++) {
      if (symbol & 1) PINB |= _BV(PB3);
      else PINB &= ~_BV(PB3);

      TIMER0_COMPA_vect();

      symbol>>=1;
    }

    mu_assert("Did not re-enable pcint",
              GIMSK & _BV(PCIE));

    mu_assert("Did not disable timer interrupt",
              !(TIMSK & _BV(OCIE0A)));

  }

  return (0);
}

static char *all_tests()
{
  mu_run_test(test_bittimes);

  mu_run_test(test_init);
  mu_run_test(test_pcint_nostartbit);
  mu_run_test(test_pcint_startbit);

  mu_run_test(test_receive);

  return 0;
}

int main (__attribute__((unused)) int argc, __attribute__((unused)) char **argv)
{
  char *result = all_tests();

  if (result != 0) {
    printf("%s\n", result);
  }
  else {
    printf("ALL TESTS PASSED\n");
  }

  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
