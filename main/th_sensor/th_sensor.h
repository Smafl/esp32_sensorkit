#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

extern float temperature;
extern float humidity;

/**
 * @brief Send temperature & humidity to a server
 */
void send_th_sensor_data(void);

/**
 * @brief Read temperature and humidity data from the sensor over I2C.
 */
void get_th_sensor_data(void);

/**
 * @brief FreeRTOS task that periodically reads sensor data, updates the display,
 *        and sends the data to the server.
 */
void th_sensor_update_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
