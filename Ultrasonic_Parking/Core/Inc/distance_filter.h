#ifndef DISTANCE_FILTER_H
#define DISTANCE_FILTER_H

#include "stm32f1xx_hal.h"

/* Xác định cảm biến cần lọc */
typedef enum
{
    FILTER_LEFT = 0,
    FILTER_RIGHT
} FilterChannel_t;


/* Khởi tạo bộ lọc */
void DistanceFilter_Init(void);

/* Thêm một mẫu khoảng cách */
void DistanceFilter_AddSample(FilterChannel_t channel,
                              float distance);

/* Xử lý 5 mẫu */
void DistanceFilter_Process(FilterChannel_t channel);

/* Lấy giá trị Median */
float DistanceFilter_GetMedian(FilterChannel_t channel);

/* Lấy giá trị trung bình sau khi loại ngoại lai */
float DistanceFilter_GetAverage(FilterChannel_t channel);

/* Lấy khoảng cách cuối cùng sau hiệu chuẩn */
float DistanceFilter_GetDistance(FilterChannel_t channel);

/* Thiết lập giá trị hiệu chuẩn */
void DistanceFilter_SetCalibration(FilterChannel_t channel,
                                   float offset);

/* Kiểm tra đã đủ 5 mẫu chưa */
uint8_t DistanceFilter_IsReady(FilterChannel_t channel);

#endif