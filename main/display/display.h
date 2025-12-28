#pragma once

#include "u8g2.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

extern u8g2_t u8g2;

/**
 * @brief Initialize u8g2 display
 */
void u8g2_display_init(void);

/**
 * @brief Draw temperature & humidity values
 */
void display_th_sensor_data(float temperature, float humidity);

#ifdef __cplusplus
}
#endif
