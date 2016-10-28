#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define BOOTLOADER_VERSION 1
#define STARTUP_TIMEOUT_US (50000)

#define TX_DDR  DDRB
#define TX_PIN  PINB
#define TX_PORT PORTB
#define TX_NUM  PB1

#define RX_DDR  DDRB
#define RX_PIN  PINB
#define RX_PORT PORTB
#define RX_NUM  PB0

#define OP_NOP 0
#define OP_ENUMERATE 1
#define OP_DEVINFO 2
#define OP_WRITE 3
#define OP_READ 4

#define BITTIME (104)
#define BITTIME_HALF (BITTIME/2)

struct {
  uint8_t addr;
  uint8_t op;
  uint16_t page_num;
  uint8_t data[SPM_PAGESIZE];
} pkg_buf;

void(*restart)(void)= (void(*)(void))(WDT_vect_num-1);

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

static inline void page_write(void)
{
  uint16_t base_addr= pkg_buf.page_num * SPM_PAGESIZE;

  boot_page_erase(base_addr);
  boot_spm_busy_wait();

  for (int i=0; i<SPM_PAGESIZE; i+=2) {
    uint16_t rel_addr= base_addr + i;

    uint16_t pgw= pkg_buf.data[i];

    pgw|= pkg_buf.data[i+1] << 8;

    boot_page_fill(rel_addr, pgw);
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

static inline void dev_info(void)
{
  pkg_buf.data[0]= BOOTLOADER_VERSION;

  pkg_buf.data[8]= SIGNATURE_0;
  pkg_buf.data[9]= SIGNATURE_1;
  pkg_buf.data[10]= SIGNATURE_2;

  uint16_t bl_addr= (uint16_t)pre_main;
  pkg_buf.data[16]= bl_addr & 0xff;
  pkg_buf.data[17]= bl_addr >> 8;
}

static inline void uart_recv(void)
{
  for(uint8_t bte=0; bte<sizeof(pkg_buf); bte++) {
    uint8_t byte= 0;

    // Wait for falling edge of start bit
    while(bit_is_set(RX_PIN, RX_NUM));

    // Skip start bit
    _delay_us(BITTIME + BITTIME_HALF);

    for(uint8_t bit=0; bit<8; bit++) {
      byte>>=1;

      if(bit_is_set(RX_PIN, RX_NUM)) {
        byte|= 0x80;
      }

      _delay_us(BITTIME);
    }

    ((uint8_t *)&pkg_buf)[bte]= byte;
  }
}

static inline void uart_send(void)
{
  for(uint8_t bte=0; bte<sizeof(pkg_buf); bte++) {
    uint8_t dat= ((uint8_t *)&pkg_buf)[bte];

    uint16_t frame= (((uint16_t)dat)<<1) | 0x200;

    for(uint8_t bit=0; bit<10; bit++) {
      if(frame & 1) TX_PORT|=  _BV(TX_NUM);
      else          TX_PORT&= ~_BV(TX_NUM);

      frame>>=1;

      _delay_us(BITTIME);
    }
  }
}

static inline void pin_init(void)
{
  DDRB= _BV(TX_NUM);
  TX_PORT= _BV(TX_NUM);
}

/*
 * Setup the watchdog to trigger after one second.
 * The watchdog interrupt vector is set to the
 * main programs start address.
 */
static inline void wdt_init(void)
{
  cli();
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR= _BV(WDIE) | _BV(WDE) | _BV(WDP2) | _BV(WDP1);
  sei();
}

/*
 * Before the first usage of the bootloader
 * we rely on the flash being filled with a
 * NOP sled that leads into the bootloader
 * section.
 */
__attribute__((OS_main)) int main(void)
{
  uint8_t addr= 0;

  pin_init();
  wdt_init();

  for(;;) {
    uart_recv();

    wdt_reset();

    if (pkg_buf.addr == addr) {
      if (pkg_buf.op == OP_ENUMERATE) {
        addr= ++pkg_buf.data[0];
      }

      if (pkg_buf.op == OP_DEVINFO) {
        dev_info();
      }

      if (pkg_buf.op == OP_WRITE) {
        page_write();
      }

      if (pkg_buf.op == OP_READ) {
        page_read();
      }
    }

    uart_send();
  }
}
