#ifndef HCSR04_H
#define HCSR04_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* Ma cua ba cam bien */
#define HC_LEFT    0U
#define HC_CENTER  1U
#define HC_RIGHT   2U
#define HC_COUNT   3U

typedef enum
{
    HCSR04_OK = 0,
    HCSR04_TIMEOUT,
    HCSR04_ECHO_HIGH,
    HCSR04_BAD_PULSE,
    HCSR04_HW_ERROR
} HCSR04_Status;

typedef struct
{
    uint32_t echo_us;
    HCSR04_Status status;
    uint8_t sensor;
} HCSR04_Result;

HAL_StatusTypeDef HCSR04_Init(void);

void HCSR04_Task(void);

uint8_t HCSR04_GetResult(HCSR04_Result *result);

void HCSR04_CaptureCallback(TIM_HandleTypeDef *htim);

void HCSR04_EXTI_Callback(uint16_t GPIO_Pin);

#endif