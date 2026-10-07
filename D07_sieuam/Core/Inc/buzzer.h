#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f1xx_hal.h"

extern volatile uint8_t buzzer_on;

HAL_StatusTypeDef Buzzer_Init(void);
void Buzzer_Task(uint8_t valid, uint16_t distance_x10);

#endif