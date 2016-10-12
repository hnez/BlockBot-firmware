#ifndef __UNIT_TEST__
  // Do not load avr headers in unit test code
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <util/atomic.h>

  #include <rdbuf.h>
#endif

#include "uart.h"

#include <string.h>
#include <stdbool.h>

#define BAUD_RATE 9600
#define UART_BITTIME(bit) (((1+2*bit)*F_CPU)/(2L*BAUD_RATE*UA_TMR_PRESCALE_NUM))

/* For ATtiny85 */
#define TX_DDR  DDRB
#define TX_PIN  PINB
#define TX_PORT PORTB
#define TX_NUM  PB1

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_PORT PORTB
#define RX_NUM  PB0

#define UA_TMR_PRESCALE_REG (_BV(CS01) | _BV(CS00))
#define UA_TMR_PRESCALE_NUM (64)
#define UA_BYTE_GAP_TIME (UART_BITTIME(9) / 2) // About half a byte of delay

#define PING_TMR_PRESCALE_REG (_BV(CS13) | _BV(CS11))
#define PING_TMR_PRESCALE_NUM (512)

#define UA_BITN_START (0)
#define UA_BITN_STOP (9)
#define UA_BITN_DATA(s) (s+1)

struct uart_t uart;

// For F_CPU ATtiny clock and prescaler value of UA_TMR_PRESCALE_NUM
const uint8_t uart_times[] PROGMEM= {
  UART_BITTIME(0),  // Start bit
  UART_BITTIME(1), UART_BITTIME(2), UART_BITTIME(3), UART_BITTIME(4), // Data bits
  UART_BITTIME(5), UART_BITTIME(6), UART_BITTIME(7), UART_BITTIME(8),
  UART_BITTIME(9), // Stop bit
};

inline void tx_set(bool high)
{
  TX_PORT= (TX_PORT & ~_BV(TX_NUM)) | (high ? _BV(TX_NUM) : 0);
}

inline bool rx_get(void)
{
  return((RX_PIN & _BV(RX_NUM)) != 0);
}

inline void bittimer_start(void)
{
  /* Shedule wakup for middle of received start bit.
   * Timer/Counter0 Output Compare Match A Interrupt Enable
   * Reset counter
   * Start counter */
  OCR0A= pgm_read_byte(&uart_times[0]);
  TIMSK|= _BV(OCIE0A);
  TCNT0= 0;
  TCCR0B= UA_TMR_PRESCALE_REG;
}

inline void bittimer_stop(void)
{
  /* Disable interrupt
   * and stop timer*/
  TIMSK&= ~_BV(OCIE0A);
  TCCR0B= 0;
}

inline void rx_interrupt_enable(void)
{
  // Pin Change Interrupt Enable for RX
  PCMSK|= _BV(RX_NUM);
  GIMSK|= _BV(PCIE);
}

inline void rx_interrupt_disable(void)
{
  GIMSK&= ~_BV(PCIE);
}

inline bool active_transmission(void)
{
  return (TIMSK & _BV(OCIE0A));
}

/**
 * RX pin toggle interrupt
 */
ISR(PCINT0_vect)
{
  if (rx_get()) {
    // RX pin went high. This is not a start bit

    return;
  }

  /* Shedule wakup for middle of received start bit. */
  bittimer_start();

  /* Inform the bit time interrupt that this is the first
   * bit in an uart datagram.
   * And that it should be read. */
  uart.bit_num= 0;
  uart.clk_active= 0;

  /* This is a dirty fix to make sure the stop bit (TX line high)
   * is sent even if the sender is in a hurry and sends the
   * next start bit a bit early */
  tx_set(true);

  /* Disable Pin change interrupts while the transmission
   * is active (The RX line is read in the bit timer interrupt) */
  GIMSK&= ~_BV(PCIE);
}

/**
 * Uart bit timer compare interrupt.
 */
ISR(TIMER0_COMPA_vect)
{
  static struct {
    uint8_t rx;
    uint8_t tx;
    uint8_t rx_en :1;
    uint8_t tx_en :1;
  } bbuf = { 0 };

  if(uart.bit_num == UA_BITN_START) {
    if(uart.cb_tx && uart.cb_tx(&bbuf.tx)) {
      /* The user has a byte to send.
       * Enable transmission*/
      bbuf.tx_en= true;
    }
    else {
      /* No Byte to send.
       * Disable transmission and leave active clocking mode
       * if it was active */
      bbuf.tx_en= false;
      uart.clk_active= 0;
    }

    /* Check if there is a Start bit on the line
     * and enable receiving when it is */
    if(!rx_get()) {
      bbuf.rx_en= true;
    }

    /* Set the TX line low (e.g. send a start bit)
     * if transmission is enabled.
     * Set it high otherwise */
    if (bbuf.tx_en) {
      tx_set(false);
    }
    else {
      tx_set(true);
    }
  }

  if(uart.bit_num >= UA_BITN_DATA(0) &&
     uart.bit_num <= UA_BITN_DATA(7)) {

    if (bbuf.rx_en) {
      /* Shift the bit on the RX line into the receive buffer.
       * The bit is inserted as the most significant bit
       * and shifted left in later iterations
       * (least significant bit first) */
      bbuf.rx= (bbuf.rx >> 1) | (rx_get() ? _BV(7) : 0);
    }

    /* Shift the least significant bit in the send buffer
     * out onto the TX line. */
    if (bbuf.tx_en) {
      tx_set(bbuf.tx & 0x01);
      bbuf.tx>>=1;
    }
  }

  if((uart.clk_passive || uart.clk_active)
     && uart.bit_num < UA_BITN_STOP) {

    /* This is not the last bit in the UART Datagram.
     * Shedule the next bit */
    uart.bit_num++;
    OCR0A= pgm_read_byte(&uart_times[uart.bit_num]);
  }
  else {
    /* This is the last bit in the UART datagram.
     * Decide if and how the next byte is sheduled */

    /* Reset the bit counter so that on the
     * next invocation the start bit will be transmitted. */
    uart.bit_num= 0;

    bittimer_stop();

    /* The byte we might have read is not valid when
     * There is no stop bit (rx high) on the line.
     * Or the clocking mode is in a disabled state.
     * Unsetting rx_en discards the byte. */
    if (!rx_get() || !(uart.clk_passive || uart.clk_active)) {
      bbuf.rx_en= false;
    }

    /* Send the stop bit */
    if (bbuf.tx_en) {
      tx_set(true);
    }

    /* Check if the user wants the byte we read */
    if(!bbuf.rx_en || !uart.cb_rx || !uart.cb_rx(bbuf.rx)) {
      uart.clk_passive= 0;
    }

    if(uart.clk_active && !uart.clk_passive) {
      /* Active clocking selected.
       * We are responsible to shedule the next byte*/

      /* Use the 8 bit overflow to wait for
       * UA_BYTE_GAP_TIME + uart_times[0]
       * timer ticks before starting the next byte*/
      TCNT0= 0xff - UA_BYTE_GAP_TIME;
      OCR0A= pgm_read_byte(&uart_times[0]);

      /* Enable this interrupt
       * Start the timer */
      TIMSK|= _BV(OCIE0A);
      TCCR0B= UA_TMR_PRESCALE_REG;
    }

    if(uart.clk_passive && !uart.clk_active) {
      /* Passive clocking selected.
       * The master is responsible to shedule the next byte*/

      /* Enable interrupt that waits for next start bit */
      rx_interrupt_enable();
    }
  }

}

/**
 * Start transmitting
 */
void uart_start_active(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    uart.clk_active= 1;
    uart.clk_passive= 0;

    /* Only start if no transmission is active */
    if (!active_transmission()) {
      /* Shedule wakup for middle of received start bit. */
      bittimer_start();
    }
  }
}

/**
 * Start waiting for a master
 * to start a transmission
 */
void uart_start_passive(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    uart.clk_active= 0;
    uart.clk_passive= 1;

    /* Only start if no transmission is active */
    if (!active_transmission()) {
      rx_interrupt_enable();
    }
  }
}

/**
 * Disable uart.
 * If a byte is being tranmitted right now
 * it will be completed before the uart is disabled.
 */
void uart_disabled_mode(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    uart.clk_active= 0;
    uart.clk_passive= 0;

    rx_interrupt_disable();
  }
}

void uart_init(void)
{
  // Set TX pin to driven high state
  TX_DDR|= _BV(TX_NUM);
  tx_set(true);

  // Disable RX pullup
  RX_DDR&= ~_BV(RX_NUM);
  RX_PORT&= ~_BV(RX_NUM);

  bittimer_stop();

  uart_disabled_mode();
}
