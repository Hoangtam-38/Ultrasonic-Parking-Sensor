#ifndef DISTANCE_SELECTOR_H
#define DISTANCE_SELECTOR_H

#include "stm32f1xx_hal.h"


/* Xác định bên có vật cản gần hơn */
typedef enum
{
    SIDE_LEFT = 0,
    SIDE_RIGHT,
    SIDE_NONE
} Side_t;


/* Khởi tạo module */
void DistanceSelector_Init(void);

/* Tìm khoảng cách nhỏ hơn giữa trái và phải */
float DistanceSelector_GetNearest(float left,
                                  float right);

/* Xác định bên gần hơn */
Side_t DistanceSelector_GetNearestSide(float left,
                                       float right);

/* Lấy khoảng cách gần nhất đã lưu */
float DistanceSelector_GetDistance(void);

/* Lấy bên gần nhất đã lưu */
Side_t DistanceSelector_GetSide(void);

/* Cập nhật dữ liệu hai cảm biến */
void DistanceSelector_Update(float left,
                             float right);

#endif