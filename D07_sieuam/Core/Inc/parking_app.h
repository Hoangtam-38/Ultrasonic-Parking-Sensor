#ifndef PARKING_APP_H
#define PARKING_APP_H
#include "stm32f1xx_hal.h"
HAL_StatusTypeDef Parking_Init(void);
void Parking_Task(void);
#endif
