#ifndef HCSR04_H
#define HCSR04_H
#include "stm32f1xx_hal.h"
#include <stdint.h>
typedef enum {
    HCSR04_OK = 0, HCSR04_TIMEOUT, HCSR04_ECHO_HIGH,
    HCSR04_BAD_PULSE, HCSR04_HW_ERROR
} HCSR04_Status;
typedef struct {
    uint32_t echo_us;
    HCSR04_Status status;
} HCSR04_Result;
HAL_StatusTypeDef HCSR04_Init(void);
void HCSR04_Task(void);
uint8_t HCSR04_GetResult(HCSR04_Result *result);
void HCSR04_CaptureCallback(TIM_HandleTypeDef *htim);
#endif
