#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SemaphoreHandle_t i2c_mutex;

extern i2c_master_bus_handle_t i2c_bus;
extern i2c_master_dev_handle_t i2c_dev_th_sensor;
extern i2c_master_dev_handle_t i2c_dev_display;
extern i2c_master_dev_handle_t i2c_dev_accelerometer;

/**
 * @brief Initialize I2C master and devices
 */
void i2c_master_init(void);
void i2c_discover(void);

#ifdef __cplusplus
}
#endif
