#include "buzzer.h"
#include "tim.h"
/* 0 = cho phep keu, 1 = tat tieng */
volatile uint8_t buzzer_muted = 0U;

static GPIO_PinState button_last;
static GPIO_PinState button_stable;
static uint32_t button_tick;

static void Buzzer_ButtonInit(void)
{
    buzzer_muted = 0U;
    button_last = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
    button_stable = button_last;
    button_tick = HAL_GetTick();
}

static void Buzzer_ButtonTask(void)
{
    GPIO_PinState raw;
    uint32_t now;

    now = HAL_GetTick();
    raw = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);

    if (raw != button_last)
    {
        button_last = raw;
        button_tick = now;
    }

    /* Chap nhan trang thai sau khi on dinh 30 ms */
    if ((raw != button_stable) &&
        ((uint32_t)(now - button_tick) >= 30U))
    {
        button_stable = raw;

        /* Chi doi trang thai mot lan khi nhan */
        if (button_stable == GPIO_PIN_SET)
        {
            buzzer_muted ^= 1U;
        }
    }
}
/* Xem trong Watch: 1 = phat xung, 0 = tat */
volatile uint8_t buzzer_on = 0U;

static uint8_t previous_level = 0U;
static uint32_t cycle_tick = 0U;

static void Buzzer_Output(uint8_t on)
{
    buzzer_on = on;

    /*
     * PWM mode 1, Polarity LOW, ARR = 499.
     * CCR = 250: phat xung, duty 50%.
     * CCR = 0: dau ra HIGH, tat coi.
     */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
                          on ? 250U : 0U);
}

HAL_StatusTypeDef Buzzer_Init(void)
{
    previous_level = 0U;
    cycle_tick = HAL_GetTick();

    Buzzer_Output(0U);
		Buzzer_ButtonInit();

    return HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void Buzzer_Task(uint8_t valid, uint16_t distance_x10)
{
    static uint8_t last_level = 0U;
    static uint8_t sounding = 0U;
    static uint32_t phase_tick = 0U;

    uint8_t level;
    uint32_t now;
    uint32_t off_time;

    /*now = HAL_GetTick();*/
    /*level = 0U;*/
	Buzzer_ButtonTask();

	now = HAL_GetTick();
	level = 0U;

/* Chi bao coi khi du lieu hop le va khong tat tieng */
if ((valid != 0U) && (buzzer_muted == 0U))

    /* Cung nguong voi 5 LED bao muc */
    /* distance_x10: 250 = 25.0 cm */
    if (valid != 0U)
    {
        if      (distance_x10 <= 200U)  level = 5U;
        else if (distance_x10 <= 400U)  level = 4U;
        else if (distance_x10 <= 600U)  level = 3U;
        else if (distance_x10 <= 800U)  level = 2U;
        else if (distance_x10 <= 1000U) level = 1U;
    }

    /* Khong co canh bao: tat coi */
    if (level == 0U)
    {
        last_level = 0U;
        sounding = 0U;
        phase_tick = now;
        Buzzer_Output(0U);
        return;
    }

    /* Sang 5 LED: coi keu lien tuc */
    if (level == 5U)
    {
        last_level = 5U;
        sounding = 1U;
        phase_tick = now;
        Buzzer_Output(1U);
        return;
    }

    /* Thoi gian nghi theo so LED dang sang */
    switch (level)
    {
        case 1U: off_time = 900U; break;
        case 2U: off_time = 600U; break;
        case 3U: off_time = 300U; break;
        default: off_time = 100U; break;
    }

    /* Vua vao vung canh bao: keu ngay */
    if (last_level == 0U)
    {
        sounding = 1U;
        phase_tick = now;
    }
    /* Roi vung keu lien tuc: bat dau bang khoang nghi */
    else if (last_level == 5U)
    {
        sounding = 0U;
        phase_tick = now;
    }

    /*
     * Doi giua muc 1..4 khong reset nhip.
     * Tranh dao dong khoang cach lam coi keu mai.
     */
    last_level = level;

    if (sounding != 0U)
    {
        /* Moi tieng bip dai 100 ms */
        if ((uint32_t)(now - phase_tick) >= 100U)
        {
            sounding = 0U;
            phase_tick = now;
        }
    }
    else
    {
        if ((uint32_t)(now - phase_tick) >= off_time)
        {
            sounding = 1U;
            phase_tick = now;
        }
    }

    Buzzer_Output(sounding);
}