#ifndef LEVEL_LED_H
#define LEVEL_LED_H
#include <stdint.h>
/* Chi dieu khien 5 LED bao muc; khong co coi hay nut bam. */
void LevelLED_Init(void);
void LevelLED_Task(uint8_t valid, uint16_t cm_x10);
uint8_t LevelLED_Level(void);
#endif
