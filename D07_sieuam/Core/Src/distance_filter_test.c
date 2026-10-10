#include "distance_filter.h"
#include <stdint.h>

volatile uint32_t df_test_done;
volatile uint32_t df_test_pass;
volatile uint32_t df_test_fail;
volatile float df_test_result;

static uint8_t Near(float actual, float expected)
{
    float error = actual - expected;
    if (error < 0.0f) error = -error;
    return (error < 0.02f) ? 1U : 0U;
}

static void Check(uint8_t passed)
{
    if (passed) df_test_pass++;
    else        df_test_fail++;
}

void DF_Test_Run(void)
{
    uint8_t i;
    uint8_t result;
    uint8_t outlier;
    float filtered;

    df_test_done = 0U;
    df_test_pass = 0U;
    df_test_fail = 0U;
    df_test_result = 0.0f;

    /* Bốn mẫu đầu chưa đủ; mẫu thứ năm phải trả median = 10 cm. */
    DF_Reset();
    for (i = 0U; i < 4U; i++) {
        result = DF_Push(10.0f, &filtered, &outlier);
        Check(result == 0U);
    }

    result = DF_Push(10.0f, &filtered, &outlier);
    Check(result == 1U);
    Check(Near(filtered, 10.0f));
    Check(outlier == 0U);

    /* Mẫu cuối là ngoại lai: được đánh dấu, median đầu ra vẫn là 10 cm. */
    DF_Reset();
    DF_Push(10.0f, &filtered, &outlier);
    DF_Push(10.0f, &filtered, &outlier);
    DF_Push(10.0f, &filtered, &outlier);
    DF_Push(10.0f, &filtered, &outlier);
    result = DF_Push(30.0f, &filtered, &outlier);

    Check(result == 1U);
    Check(Near(filtered, 10.0f));
    Check(outlier == 1U);

    /* Median lần lượt là 10, 10, 20; trung bình cuối = 13.33 cm. */
    DF_Reset();
    for (i = 0U; i < 5U; i++) {
        result = DF_Push(10.0f, &filtered, &outlier);
    }
    Check(result == 1U);
    Check(Near(filtered, 10.0f));

    DF_Push(20.0f, &filtered, &outlier);
    DF_Push(20.0f, &filtered, &outlier);
    DF_Push(20.0f, &filtered, &outlier);
    df_test_result = filtered;
    Check(Near(filtered, 13.3333f));

    /* Ngoài phạm vi 2–400 cm phải bị từ chối. */
    DF_Reset();
    Check(DF_Push(1.9f, &filtered, &outlier) == 0U);
    Check(DF_Push(400.1f, &filtered, &outlier) == 0U);
    Check(DF_Push(10.0f, 0, &outlier) == 0U);
    Check(DF_Push(10.0f, &filtered, 0) == 0U);

    df_test_done = 1U;
}