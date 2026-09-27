#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "stm32f1xx_hal.h"

typedef enum
{
    ULTRASONIC_LEFT = 0,
    ULTRASONIC_RIGHT
} UltrasonicSensor_t;

/* Khởi tạo module */
void Ultrasonic_Init(TIM_HandleTypeDef *htim);

/* Tạo xung Trigger */
void Ultrasonic_Trigger(UltrasonicSensor_t sensor);

/* Xử lý Input Capture */
void Ultrasonic_CaptureCallback(TIM_HandleTypeDef *htim);

/* Lấy khoảng cách thô */
float Ultrasonic_GetRawDistance(UltrasonicSensor_t sensor);

/* Kiểm tra timeout */
uint8_t Ultrasonic_IsTimeout(UltrasonicSensor_t sensor);

#endif