#pragma once

#include <vm.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define MOT1_ADDR 0
#define MOT2_ADDR 1

#define MOT_PWM_TOP 255
#define MOT_PWM_PRESCALE 8

uint8_t mem_getmot (struct vm_status_t *, uint8_t, uint8_t *);
uint8_t mem_setmot (struct vm_status_t *, uint8_t, uint8_t);
void motor_init (void);
