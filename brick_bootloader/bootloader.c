#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define TX_DDR  DDRB
#define TX_PIN  PINB
#define TX_PORT PORTB
#define TX_NUM  PB1

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_PORT PORTB
#define RX_NUM  PB0

#define OP_NOP 0
#define OP_WRITE 1
#define OP_READ 2
#define OP_RESTART 3

struct {
  uint8_t op;
  uint16_t page_num;
  uint8_t data[SPM_PAGESIZE];
} pkg_buf;

void(*restart)(void)= NULL;

static inline void page_write(void)
{
  uint16_t base_addr= pkg_buf.page_num * SPM_PAGESIZE;

  boot_page_erase(base_addr);
  boot_spm_busy_wait();

  for (int i=0; i<SPM_PAGESIZE; i+=2) {
    uint16_t pgw= pkg_buf.data[i];
    pgw|= pkg_buf.data[i+1] << 8;

    boot_page_fill(base_addr + i, pgw);
  }

  boot_page_write(base_addr);
  boot_spm_busy_wait();
}

static inline void page_read(void)
{
  uint16_t base_addr= pkg_buf.page_num * SPM_PAGESIZE;

  for (int i=0; i<SPM_PAGESIZE; i+=2) {
    uint16_t rdw= pgm_read_word(base_addr + i);

    pkg_buf.data[i]= rdw & 0xff;
    pkg_buf.data[i+1]= rdw >> 8;
  }
}

static inline void led_setup(void)
{
  DDRB= _BV(PB3) | _BV(PB4);
}

static inline void led_notify(void)
{
  PORTB= _BV(PB4);
  _delay_ms(50);

  PORTB= 0;
  _delay_ms(100);

  PORTB= _BV(PB4);
  _delay_ms(50);

  PORTB= 0;
}

static inline void uart_rcv(void)
{
  uint8_t *buf= (void*)&pkg_buf;
  uint8_t byte= 0;

  for(uint8_t i=0; i<sizeof(pkg_buf); i++) {
    /* Wait for start of byte */
    while(RX_PIN & _BV(RX_NUM));

    _delay_us(52+104);

    for(uint8_t bit=0; bit<8; bit++) {
      byte= (byte>>1) | ((RX_PIN & _BV(RX_NUM)) ? _BV(7) : 0);

      if (bit==7) _delay_us(52);
      else _delay_us(104);
    }

    buf[i]= byte;
  }
}

static inline void uart_send(void)
{
  uint8_t *buf= (void*)&pkg_buf;
  uint8_t byte;

  for(uint8_t i=0; i<sizeof(pkg_buf); i++) {
    byte= buf[i];

    TX_PORT&= ~_BV(TX_NUM);
    _delay_us(104);

    for(uint8_t bit=0; bit<8; bit++) {
      if (byte & 0x01) {
        TX_PORT|= _BV(TX_NUM);
      }
      else {
        TX_PORT&= ~_BV(TX_NUM);
      }

      byte>>=1;
      _delay_us(104);
    }

    TX_PORT|= _BV(TX_NUM);
    _delay_us(104);

    buf[i]= byte;
  }
}

static inline void uart_init(void)
{
  TX_DDR|= _BV(TX_NUM);
  TX_PORT|= _BV(TX_NUM);
}

/*
 * Do the minimal necessarry setup before executing main.
 * (Setup stack pointer, clear zero register)
 */
__attribute__((naked)) __attribute__((section (".init9"))) void pre_main (void)
{
  register uint8_t spl= RAMEND & 0xff;
  register uint8_t sph= RAMEND>>8;

  asm volatile("out __SP_L__, %0\n"
               "out __SP_H__, %1\n"
               :: "r" (spl), "r" (sph));

  asm volatile ( "clr __zero_reg__" );
  asm volatile ( "rjmp main");
}

/*
 * Before the first usage of the bootloader
 * we rely on the flash being filled with a
 * NOP sled that leads into the bootloader
 * section.
 */
__attribute__((OS_main))int main(void)
{
  led_setup();
  led_notify();
  uart_init();

  while(pkg_buf.op != OP_RESTART) {
    uart_rcv();

    if (pkg_buf.op == OP_WRITE) {
      page_write();
    }

    if (pkg_buf.op == OP_READ) {
      page_read();
    }

    uart_send();
  }

  restart();
}
