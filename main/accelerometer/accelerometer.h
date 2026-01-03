#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

extern float x_acc;
extern float y_acc;
extern float z_acc;

/**
 * @brief FreeRTOS task that periodically reads accelerometer data.
 */
void accelerometer_update_task(void *pvParameters);

/**
 * @brief Read accelerometer data over I2C.
 */
void get_accelerometer_data();

/**
 * @brief Initialize the LIS3DH accelerometer.
 */
void accelerometer_sensor_init(void);

#ifdef __cplusplus
}
#endif
