#include "distance_filter.h"

static float samples[5], averages[3];
static uint8_t count, index_in, avg_count, avg_index;

static float AbsF(float x) { return x < 0.0f ? -x : x; }

void DF_Reset(void)
{
    count = 0U;
    index_in = 0U;
    avg_count = 0U;
    avg_index = 0U;
}

uint8_t DF_Push(float raw_cm, float *filtered_cm, uint8_t *outlier)
{
    float sorted[5], key, median, gate, sum;
    uint8_t i, j;
    if (filtered_cm == 0 || outlier == 0) return 0U;
    *outlier = 0U;
    /* Cach so sanh nay cung loai NaN */
    if (!(raw_cm >= 2.0f && raw_cm <= 400.0f)) return 0U;
    samples[index_in] = raw_cm;
    index_in = (uint8_t)((index_in + 1U) % 5U);
    if (count < 5U) count++;
    if (count < 5U) return 0U;
    for (i = 0U; i < 5U; i++) sorted[i] = samples[i];
    for (i = 1U; i < 5U; i++) {
        key = sorted[i];
        j = i;
        while (j > 0U && sorted[j - 1U] > key) {
            sorted[j] = sorted[j - 1U];
            j--;
        }
        sorted[j] = key;
    }
    median = sorted[2];
    gate = 0.15f * median;
    if (gate < 3.0f) gate = 3.0f;
    if (AbsF(raw_cm - median) > gate) *outlier = 1U;
    /* Mau moi luon vao cua so: khong khoa bo loc khi vat doi vi tri.
       Duong ra dung median, khong dung truc tiep mau ngoai lai. */
    averages[avg_index] = median;
    avg_index = (uint8_t)((avg_index + 1U) % 3U);
    if (avg_count < 3U) avg_count++;
    sum = 0.0f;
    for (i = 0U; i < avg_count; i++) sum += averages[i];
    *filtered_cm = sum / (float)avg_count;
    return 1U;
}
