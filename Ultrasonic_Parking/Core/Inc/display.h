#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f1xx_hal.h"


/* Xác định bên được hiển thị */
typedef enum
{
    DISPLAY_LEFT = 0,
    DISPLAY_RIGHT,
    DISPLAY_NONE
} DisplaySide_t;


/* Khởi tạo module hiển thị */
void Display_Init(void);

/* Cập nhật khoảng cách cần hiển thị */
void Display_SetDistance(float distance);

/* Cập nhật bên có vật cản gần hơn */
void Display_SetSide(DisplaySide_t side);

/* Cập nhật mức thanh LED */
void Display_SetLevel(uint8_t level);

/* Quét LED 7 đoạn */
void Display_Scan(void);

/* Xóa màn hình */
void Display_Clear(void);

#endif