#include <avr/io.h>

#include "vm.h"

#include "motor.h"

#define MOT1A_DDR  DDRD
#define MOT1A_PORT PORTD
#define MOT1A_NUM  PD6
#define MOT1A_PWM  OCR0B
#define MOT1A_INV  COM0A0

#define MOT1B_DDR  DDRC
#define MOT1B_PORT PORTC
#define MOT1B_NUM  PC2

#define MOT2A_DDR  DDRD
#define MOT2A_PORT PORTD
#define MOT2A_NUM  PD5
#define MOT2A_PWM  OCR0A
#define MOT2A_INV  COM0B0

#define MOT2B_DDR  DDRC
#define MOT2B_PORT PORTC
#define MOT2B_NUM  PC1

uint8_t mem_getmot (__attribute__((unused)) struct vm_status_t *vm,
                    uint8_t addr, uint8_t *val)
{
  if (addr==MOT1_ADDR) {
    *val= (TCCR0A & _BV(MOT1A_INV)) ? -(int8_t)MOT1A_PWM : MOT1A_PWM;

    return (MEM_OK);
  }
  else if (addr==MOT2_ADDR) {
    *val= (TCCR0A & _BV(MOT2A_INV)) ? -(int8_t)MOT2A_PWM : MOT2A_PWM;

    return (MEM_OK);
  }
  else {
    return (MEM_ERR);
  }
}

uint8_t mem_setmot (__attribute__((unused)) struct vm_status_t *vm,
                    uint8_t addr, uint8_t val)
{
  register int8_t speed= (int8_t)val;

  /* Positive numbers can be encoded up to 127, which is also the top of the PWM
   * Negative numbers can be encoded down to -128 which is one to many for the PWM */
  register uint8_t absspeed= (speed >= 0) ? speed : ((speed == -128) ? 127 : -speed);

  if (addr==MOT1_ADDR) {
    if (speed >= 0) {
      MOT1B_PORT|=  _BV(MOT1B_NUM);
      TCCR0A|= _BV(MOT1A_INV); // invert PWM
    }
    else {
      MOT1B_PORT&= ~_BV(MOT1B_NUM);
      TCCR0A&=~_BV(MOT1A_INV); // normal PWM
    }

    MOT1A_PWM= absspeed * 2;

    return (MEM_OK);
  }
  else if (addr==MOT2_ADDR) {
    if (speed >= 0) {
      MOT2B_PORT|=  _BV(MOT2B_NUM);
      TCCR0A|= _BV(MOT2A_INV); // invert PWM
    }
    else {
      MOT2B_PORT&= ~_BV(MOT2B_NUM);
      TCCR0A&=~_BV(MOT2A_INV); // normal PWM
    }

    MOT2A_PWM= absspeed * 2;

    return (MEM_OK);
  }
  else {
    return (MEM_ERR);
  }
}


void motor_init (void)
{
  mem_setmot(NULL, MOT1_ADDR, 0);
  mem_setmot(NULL, MOT2_ADDR, 0);

  TCCR0A = _BV(COM0A1) | _BV(COM0B1)
    |_BV(WGM01) | _BV(WGM00);

  TCCR0B = _BV(CS01);

  MOT1A_DDR|= _BV(MOT1A_NUM);
  MOT1B_DDR|= _BV(MOT1B_NUM);

  MOT2A_DDR|= _BV(MOT2A_NUM);
  MOT2B_DDR|= _BV(MOT2B_NUM);
}
