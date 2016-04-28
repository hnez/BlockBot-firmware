#include <avr/io.h>
#include "motor.h"


#define MOT1A_DDR  DDRA
#define MOT1A_PORT PORTA
#define MOT1A_NUM  PA7
#define MOT1A_PWM  OCR0B

#define MOT1B_DDR  DDRA
#define MOT1B_PORT PORTA
#define MOT1B_NUM  PA0

#define MOT2A_DDR  DDRB
#define MOT2A_PORT PORTB
#define MOT2A_NUM  PB2
#define MOT2A_PWM  OCR0A

#define MOT2B_DDR  DDRA
#define MOT2B_PORT PORTA
#define MOT2B_NUM  PA1

void motor_1_set (int16_t speed)
{
  if (speed >= 0) {
    MOT1B_PORT|=  _BV(MOT1B_NUM);

    MOT1A_PWM=255-speed;
  }
  else {
    MOT1B_PORT&= ~_BV(MOT1B_NUM);

    MOT1A_PWM=-speed;
  }
}

void motor_2_set (int16_t speed)
{
  if (speed >= 0) {
    MOT2B_PORT|=  _BV(MOT2B_NUM);

    MOT2A_PWM=255-speed;
  }
  else {
    MOT2B_PORT&= ~_BV(MOT2B_NUM);

    MOT2A_PWM=-speed;
  }
}

void motor_init (void)
{

  motor_1_set(0);
  motor_2_set(0);

  TCCR0A = _BV(COM0A1) | _BV(COM0B1)
    |_BV(WGM01) | _BV(WGM00);

  TCCR0B = _BV(CS00);
  
  MOT1A_DDR|= _BV(MOT1A_NUM);
  MOT1B_DDR|= _BV(MOT1B_NUM);

  MOT2A_DDR|= _BV(MOT2A_NUM);
  MOT2B_DDR|= _BV(MOT2B_NUM);
}
