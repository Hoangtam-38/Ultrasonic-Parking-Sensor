#ifndef DISTANCE_FILTER_H
#define DISTANCE_FILTER_H
#include <stdint.h>
void DF_Reset(void);
/* 1: du 5 mau va co ket qua; 0: dang khoi dong/du lieu loi */
uint8_t DF_Push(float raw_cm, float *filtered_cm, uint8_t *outlier);
#endif
