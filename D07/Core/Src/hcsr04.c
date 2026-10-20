#include "hcsr04.h"
#include "tim.h"

#define MEASURE_PERIOD_MS  70U
#define ECHO_TIMEOUT_MS    35U

/* Gioi han rong de loai xung loi ro rang */
#define MIN_ECHO_US       100U
#define MAX_ECHO_US     25000U

typedef enum
{
    STATE_IDLE = 0,
    STATE_WAIT_RISE,
    STATE_WAIT_FALL
} MeasureState;

static volatile MeasureState state = STATE_IDLE;

static volatile uint16_t rise_capture = 0U;
static volatile uint32_t shot_tick = 0U;

static uint32_t last_shot_tick = 0U;

static volatile uint8_t result_ready = 0U;
static volatile uint32_t result_echo_us = 0U;
static volatile HCSR04_Status result_status = HCSR04_TIMEOUT;

static volatile uint8_t current_sensor = HC_LEFT;

static GPIO_TypeDef * const echo_port[HC_COUNT] =
{
    GPIOB, GPIOA, GPIOB
};

static const uint16_t echo_pin[HC_COUNT] =
{
    GPIO_PIN_3, GPIO_PIN_6, GPIO_PIN_4
};
/* Bao ve du lieu dung chung giua main va ngat */
static uint32_t LockIRQ(void)
{
    uint32_t key = __get_PRIMASK();
    __disable_irq();
    return key;
}

static void UnlockIRQ(uint32_t key)
{
    __set_PRIMASK(key);
}


/* Goi trong ngat hoac khi da khoa ngat */
static void FinishMeasurement(HCSR04_Status status,
                              uint32_t echo_us)
{
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_CC1);

    state = STATE_IDLE;

    result_echo_us = echo_us;
    result_status = status;
    result_ready = 1U;
}
static HAL_StatusTypeDef SendSideTrigger(uint16_t pin)
{
    uint16_t start;
    uint32_t guard = 100000U;

    HAL_GPIO_WritePin(GPIOA, pin, GPIO_PIN_SET);
    start = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);

    /* TIM3: 1 tick = 1 us */
    while ((uint16_t)(
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim3) - start
    ) < 12U)
    {
        /* Tranh ket neu timer khong chay */
        if (--guard == 0U)
        {
            HAL_GPIO_WritePin(GPIOA, pin, GPIO_PIN_RESET);
            return HAL_ERROR;
        }
    }

    HAL_GPIO_WritePin(GPIOA, pin, GPIO_PIN_RESET);
    return HAL_OK;
}

/* TIM2 tao xung Trigger bang phan cung */
static HAL_StatusTypeDef SendTrigger(void)
{
    if (current_sensor == HC_LEFT)
    {
        return SendSideTrigger(GPIO_PIN_0);
    }

    if (current_sensor == HC_RIGHT)
    {
        return SendSideTrigger(GPIO_PIN_15);
    }

    /* Cam bien giua: giu TRIG bang TIM2 CH2 */
    if (HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2) != HAL_OK)
    {
        return HAL_ERROR;
    }

    __HAL_TIM_SET_COUNTER(&htim2, 0U);

    htim2.Instance->EGR = TIM_EGR_UG;

    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

    return HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}


HAL_StatusTypeDef HCSR04_Init(void)
{
	current_sensor = HC_LEFT;

HAL_GPIO_WritePin(
    GPIOA,
    GPIO_PIN_0 | GPIO_PIN_15,
    GPIO_PIN_RESET
);

__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3 | GPIO_PIN_4);
    state = STATE_IDLE;
    result_ready = 0U;
    result_echo_us = 0U;
    result_status = HCSR04_TIMEOUT;

    /*
     * TIM2:
     * PSC = 71, ARR = 12, PWM mode 2, CCR2 = 1.
     * OPM: tu dung khi co update event.
     */
    SET_BIT(htim2.Instance->CR1, TIM_CR1_OPM);

    __HAL_TIM_SET_COUNTER(&htim2, 0U);
    htim2.Instance->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

    /* TIM3 chay lien tuc voi 1 tick = 1 us */
    __HAL_TIM_SET_COUNTER(&htim3, 0U);

    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim3,
        TIM_CHANNEL_1,
        TIM_INPUTCHANNELPOLARITY_RISING
    );

    if (HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Chi bat ngat capture khi bat dau mot lan do */
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_CC1);

    __HAL_TIM_CLEAR_FLAG(
        &htim3,
        TIM_FLAG_CC1 | TIM_FLAG_CC1OF
    );

    /* Cho cam bien on dinh 70 ms truoc lan do dau */
    last_shot_tick = HAL_GetTick();

    return HAL_OK;
}


void HCSR04_Task(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t key = LockIRQ();

    /* Timeout khi khong co canh len hoac khong co canh xuong */
    if ((state != STATE_IDLE) &&
        ((uint32_t)(now - shot_tick) >= ECHO_TIMEOUT_MS))
    {
        FinishMeasurement(HCSR04_TIMEOUT, 0U);
    }

    /*
     * Khong ghi de ket qua chua duoc ung dung doc.
     * Moi lan kich cach nhau it nhat 70 ms.
     */
    if ((state == STATE_IDLE) &&
        (result_ready == 0U) &&
        ((uint32_t)(now - last_shot_tick) >= MEASURE_PERIOD_MS))
    {
        last_shot_tick = now;

        /*
         * Echo phai thap truoc khi kich.
         * Neu dang cao: bao loi va thu lai o chu ky sau.
         */
        if (HAL_GPIO_ReadPin(echo_port[current_sensor], echo_pin[current_sensor]) == GPIO_PIN_SET)
        {
            FinishMeasurement(HCSR04_ECHO_HIGH, 0U);
        }
        else
        {
            __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_CC1);

            __HAL_TIM_SET_CAPTUREPOLARITY(
                &htim3,
                TIM_CHANNEL_1,
                TIM_INPUTCHANNELPOLARITY_RISING
            );

            __HAL_TIM_CLEAR_FLAG(
                &htim3,
                TIM_FLAG_CC1 | TIM_FLAG_CC1OF
            );

            shot_tick = now;
            state = STATE_WAIT_RISE;

            /* San sang bat Echo truoc khi phat Trigger */
           if (current_sensor == HC_CENTER)
{
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_CC1);
}
else
{
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_CC1);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3 | GPIO_PIN_4);
}

            if (SendTrigger() != HAL_OK)
            {
                FinishMeasurement(HCSR04_HW_ERROR, 0U);
            }
        }
    }

    UnlockIRQ(key);
}


void HCSR04_CaptureCallback(TIM_HandleTypeDef *htim)
{
    uint16_t captured;
    uint32_t width_us;
	/* TIM3 CH1 chi xu ly Echo cua cam bien giua */
if (current_sensor != HC_CENTER)
{
    return;
}

    if ((htim->Instance != TIM3) ||
        (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1))
    {
        return;
    }

    if (state == STATE_IDLE)
    {
        return;
    }

    /* Khong chap nhan Echo den sau gioi han timeout */
    if ((uint32_t)(HAL_GetTick() - shot_tick) >= ECHO_TIMEOUT_MS)
    {
        FinishMeasurement(HCSR04_TIMEOUT, 0U);
        return;
    }

    /* Overcapture: co canh moi truoc khi xu ly xong canh cu */
    if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1OF) != RESET)
    {
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1OF);
        FinishMeasurement(HCSR04_BAD_PULSE, 0U);
        return;
    }

    captured = (uint16_t)HAL_TIM_ReadCapturedValue(
        htim,
        TIM_CHANNEL_1
    );

    if (state == STATE_WAIT_RISE)
    {
        rise_capture = captured;
        state = STATE_WAIT_FALL;

        /* Da bat canh len, chuyen sang bat canh xuong */
        __HAL_TIM_SET_CAPTUREPOLARITY(
            htim,
            TIM_CHANNEL_1,
            TIM_INPUTCHANNELPOLARITY_FALLING
        );
    }
    else if (state == STATE_WAIT_FALL)
    {
        /*
         * TIM3 co ARR = 65535.
         * Ep uint16_t xu ly phep tru modulo 65536,
         * ke ca khi bo dem tran giua hai canh.
         */
        width_us = (uint16_t)(captured - rise_capture);

        if ((width_us < MIN_ECHO_US) ||
            (width_us > MAX_ECHO_US))
        {
            FinishMeasurement(HCSR04_BAD_PULSE, width_us);
        }
        else
        {
            FinishMeasurement(HCSR04_OK, width_us);
        }
    }
}


uint8_t HCSR04_GetResult(HCSR04_Result *result)
{
    uint32_t key;

    if (result == NULL)
    {
        return 0U;
    }

    key = LockIRQ();

    if (result_ready == 0U)
    {
        UnlockIRQ(key);
        return 0U;
    }

    result->echo_us = result_echo_us;
		result->status = result_status;
		result->sensor = current_sensor;

		result_ready = 0U;

/* Chuyen sang cam bien tiep theo */
		current_sensor = (uint8_t)((current_sensor + 1U) % HC_COUNT);

    UnlockIRQ(key);

    return 1U;
}
void HCSR04_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint16_t captured;
    uint32_t width_us;
    GPIO_PinState level;

    captured = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);

    if (current_sensor == HC_CENTER)
    {
        return;
    }

    if (GPIO_Pin != echo_pin[current_sensor])
    {
        return;
    }

    if (state == STATE_IDLE)
    {
        return;
    }

    if ((uint32_t)(HAL_GetTick() - shot_tick) >= ECHO_TIMEOUT_MS)
    {
        FinishMeasurement(HCSR04_TIMEOUT, 0U);
        return;
    }

    level = HAL_GPIO_ReadPin(GPIOB, GPIO_Pin);

    if ((state == STATE_WAIT_RISE) &&
        (level == GPIO_PIN_SET))
    {
        rise_capture = captured;
        state = STATE_WAIT_FALL;
    }
    else if ((state == STATE_WAIT_FALL) &&
             (level == GPIO_PIN_RESET))
    {
        /* Xu ly duoc ca khi TIM3 tran bo dem */
        width_us = (uint16_t)(captured - rise_capture);

        if ((width_us < MIN_ECHO_US) ||
            (width_us > MAX_ECHO_US))
        {
            FinishMeasurement(HCSR04_BAD_PULSE, width_us);
        }
        else
        {
            FinishMeasurement(HCSR04_OK, width_us);
        }
    }
}