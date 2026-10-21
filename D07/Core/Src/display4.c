#include "display4.h"
#include "stm32f1xx_hal.h"

#define SEG_MASK (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10)
#define DIG_MASK (GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15)
static const uint16_t segment_pin[8] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_5, GPIO_PIN_6,
    GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10
};
static const uint16_t digit_pin[4] = {
    GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};
/* Bit 0..7 = a,b,c,d,e,f,g,dp; bit 1 nghia la can sang */
static const uint8_t font[10] = {
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F
};
static volatile uint32_t pending_frame = 0x40404040UL;
static uint32_t active_frame;
static uint8_t digit, phase;

void Display4_Init(void)
{
    HAL_GPIO_WritePin(GPIOB, DIG_MASK|SEG_MASK, GPIO_PIN_SET);
    pending_frame = 0x40404040UL;
    active_frame = pending_frame;
    digit = 0U;
    phase = 0U;
}

void Display4_ShowInvalid(void) { pending_frame = 0x40404040UL; }

void Display4_ShowTenths(uint16_t x)
{
    uint8_t a, b, c, d;
    if (x > 9999U) { Display4_ShowInvalid(); return; }
    a = x >= 1000U ? font[x / 1000U] : 0U;
    b = x >= 100U ? font[(x / 100U) % 10U] : 0U;
    c = (uint8_t)(font[(x / 10U) % 10U] | 0x80U);
    d = font[x % 10U];
    /* Mot lan ghi 32 bit, frame chi doi tai dau chu ky quet */
    pending_frame = (uint32_t)a | ((uint32_t)b << 8U)
                  | ((uint32_t)c << 16U) | ((uint32_t)d << 24U);
}

void Display4_ScanISR(void)
{
    uint8_t mask, i;
    uint16_t low_pins = 0U;
    if (phase == 0U) {
        /* Tat digit truoc khi doi segment. Khoang tat 500 us
           giup transistor PNP tat han, tranh bong mo. */
        HAL_GPIO_WritePin(GPIOB, DIG_MASK, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_MASK, GPIO_PIN_SET);
        if (digit == 0U) active_frame = pending_frame;
        mask = (uint8_t)(active_frame >> (8U * digit));
        for (i = 0U; i < 8U; i++) {
            if ((mask & (1U << i)) != 0U) low_pins |= segment_pin[i];
        }
        HAL_GPIO_WritePin(GPIOB, low_pins, GPIO_PIN_RESET);
        phase = 1U;
    } else {
        HAL_GPIO_WritePin(GPIOB, digit_pin[digit], GPIO_PIN_RESET);
        digit = (uint8_t)((digit + 1U) % 4U);
        phase = 0U;
    }
}
