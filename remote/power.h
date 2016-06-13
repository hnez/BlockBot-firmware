#pragma once

#define POWER_ON 1
#define POWER_OFF 0

void pwr_self(uint8_t);
void pwr_bricks (uint8_t);
uint8_t pwr_chkovc (void);
