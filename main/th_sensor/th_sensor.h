#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the temperature & humidity sensor update task
 */
void th_sensor_start_task(void);

#ifdef __cplusplus
}
#endif
