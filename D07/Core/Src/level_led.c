#include "level_led.h"
#include "stm32f1xx_hal.h"

static uint8_t level;

static void Bar(uint8_t n)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, n >= 1U ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, n >= 2U ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, n >= 3U ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, n >= 4U ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, n >= 5U ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LevelLED_Init(void)
{
    level = 0U;
    Bar(0U);
}

uint8_t LevelLED_Level(void) { return level; }

void LevelLED_Task(uint8_t valid, uint16_t d)
{
    /* d co don vi 0.1 cm; cac LED sang tich luy. */
    if (!valid || d > 1000U) level = 0U;
    else if (d > 800U) level = 1U;
    else if (d > 600U) level = 2U;
    else if (d > 400U) level = 3U;
    else if (d > 200U) level = 4U;
    else level = 5U;
    Bar(level);
}
