#ifndef DISPLAY4_H
#define DISPLAY4_H
#include <stdint.h>
void Display4_Init(void);
void Display4_ShowTenths(uint16_t cm_x10);
void Display4_ShowInvalid(void);
/* Goi trong ngat TIM4 moi 500 us (2 kHz) */
void Display4_ScanISR(void);
#endif
