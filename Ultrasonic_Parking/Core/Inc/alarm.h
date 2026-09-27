#ifndef ALARM_H
#define ALARM_H

#include "stm32f1xx_hal.h"


/* Khởi tạo module cảnh báo */
void Alarm_Init(TIM_HandleTypeDef *htim_pwm,
                TIM_HandleTypeDef *htim_update,
                UART_HandleTypeDef *huart);

/* Cập nhật khoảng cách gần nhất */
void Alarm_SetDistance(float distance);

/* Cập nhật trạng thái Mute */
void Alarm_SetMute(uint8_t mute);

/* Lấy trạng thái Mute */
uint8_t Alarm_IsMuted(void);

/* Cập nhật nhịp còi */
void Alarm_Update(void);

/* Gửi khoảng cách qua UART */
void Alarm_SendDistance(float distance);

/* Gửi trạng thái cảnh báo qua UART */
void Alarm_SendStatus(uint8_t level);

#endif