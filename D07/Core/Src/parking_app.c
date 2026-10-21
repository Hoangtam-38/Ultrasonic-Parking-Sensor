#include "parking_app.h"
#include "parking_config.h"
#include "hcsr04.h"
#include "distance_filter.h"
#include "display4.h"
#include "level_led.h"
#include "tim.h"

/* Khac ten debug_... cu de tranh trung dinh nghia */
volatile uint32_t park_echo_us, park_status, park_samples, park_errors;
volatile uint32_t park_outliers, park_valid, park_level;
volatile float park_raw_cm = -1.0f, park_filtered_cm = -1.0f;
static uint16_t distance_x10;
static uint32_t last_good;

static void Invalidate(void)
{
    park_valid = 0U;
    park_filtered_cm = -1.0f;
    distance_x10 = 0U;
    DF_Reset();
    Display4_ShowInvalid();
}

HAL_StatusTypeDef Parking_Init(void)
{
    Display4_Init();
    DF_Reset();
    LevelLED_Init();
    Invalidate();
    last_good = HAL_GetTick();
    if (HCSR04_Init() != HAL_OK) return HAL_ERROR;
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
    return HAL_TIM_Base_Start_IT(&htim4);
}

void Parking_Task(void)
{
    HCSR04_Result r;
    float raw, filtered, calibrated;
    uint8_t outlier;
    uint32_t now;
    HCSR04_Task();
    now = HAL_GetTick();
    if (HCSR04_GetResult(&r)) {
        park_samples++;
        park_echo_us = r.echo_us;
        park_status = (uint32_t)r.status;
        if (r.status == HCSR04_OK) {
            raw = (float)r.echo_us * (331.3f + 0.606f * PARKING_TEMP_C) / 20000.0f;
            park_raw_cm = raw;
            if (raw >= 2.0f && raw <= 400.0f) {
                last_good = now;
                if (DF_Push(raw, &filtered, &outlier)) {
                    if (outlier) park_outliers++;
                    calibrated = PARKING_CAL_A * filtered + PARKING_CAL_B_CM;
                    if (calibrated >= 0.0f && calibrated <= 400.0f) {
                        park_filtered_cm = calibrated;
                        distance_x10 = (uint16_t)(calibrated * 10.0f + 0.5f);
                        park_valid = 1U;
                        Display4_ShowTenths(distance_x10);
                    } else { park_errors++; Invalidate(); }
                }
            } else { park_errors++; Invalidate(); }
        } else {
            park_raw_cm = -1.0f;
            park_errors++;
            Invalidate();
        }
    }
    if ((uint32_t)(now - last_good) >= 300U) Invalidate();
    LevelLED_Task((uint8_t)park_valid, distance_x10);
    park_level = LevelLED_Level();
}
