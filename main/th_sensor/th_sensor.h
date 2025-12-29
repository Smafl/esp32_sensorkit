#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

extern float temperature;
extern float humidity;

/**
 * @brief Start the temperature & humidity sensor update task
 */
void th_sensor_start_task(void);

/**
 * @brief Send temperature & humidity to a server
 */
void send_th_sensor_data(void);

#ifdef __cplusplus
}
#endif
