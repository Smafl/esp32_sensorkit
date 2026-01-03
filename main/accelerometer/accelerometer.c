#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "accelerometer.h"
#include "i2c_bus/i2c_bus.h"
#include "esp_log.h"

static const char *TAG = "accelerometer";

#define LIS3DH_REG_CTRL4   0x23
#define LIS3DH_REG_OUT_X_L 0x28

float x_g = 0;
float y_g = 0;
float z_g = 0;

/**
 * @brief Read one or more registers from the LIS3DH accelerometer.
 *
 * This function performs an I2C read with automatic register address increment.
 * It uses the global `i2c_dev_accelerometer` handle. It does NOT block other
 * I2C devices automatically, so you should take `i2c_mutex` before calling it.
 *
 * @param reg  The starting register address to read from.
 * @param data Pointer to a buffer where read bytes will be stored.
 * @param len  Number of bytes to read.
 *
 * @return ESP_OK on success, or an esp_err_t error code on failure.
 */
static esp_err_t lis3dh_read(uint8_t reg, uint8_t *data, size_t len)
{
    reg |= 0x80; // auto-increment
    return i2c_master_transmit_receive(
        i2c_dev_accelerometer,
        &reg, 1,
        data, len,
        50
    );
}

/**
 * @brief Read accelerometer data over I2C.
 *
 * Updates the global variables `x_acc`, `y_acc`, `z_acc` in g units.
 */
void get_accelerometer_data(void)
{
    uint8_t ctrl4;
    if (lis3dh_read(LIS3DH_REG_CTRL4, &ctrl4, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL_REG4");
        return;
    }

    uint8_t range_bits = (ctrl4 >> 4) & 0x03; // FS1:FS0
    uint8_t hr_bit = (ctrl4 >> 3) & 0x01; // HR

    float range_g;
    switch(range_bits) {
        case 0: range_g = 2; break;   // ±2g
        case 1: range_g = 4; break;   // ±4g
        case 2: range_g = 8; break;   // ±8g
        case 3: range_g = 16; break;  // ±16g
        default: range_g = 2; break;
    }

    int bits = hr_bit ? 12 : 10;

    uint8_t raw[6];
    if (lis3dh_read(LIS3DH_REG_OUT_X_L, raw, 6) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read acceleration data");
        return;
    }

    int16_t x_raw = (int16_t)(raw[0] | (raw[1] << 8));
    int16_t y_raw = (int16_t)(raw[2] | (raw[3] << 8));
    int16_t z_raw = (int16_t)(raw[4] | (raw[5] << 8));

    if (bits == 12) {
        x_g = (float)x_raw * range_g / 2048.0f;
        y_g = (float)y_raw * range_g / 2048.0f;
        z_g = (float)z_raw * range_g / 2048.0f;
    } else {
        x_g = (float)x_raw * range_g / 512.0f;
        y_g = (float)y_raw * range_g / 512.0f;
        z_g = (float)z_raw * range_g / 512.0f;
    }
}

/**
 * @brief FreeRTOS task that periodically reads accelerometer data.
 *
 * The task loops forever with a delay between readings.
 *
 * @param pvParameters FreeRTOS task parameter (not used).
 */
void accelerometer_update_task(void *pvParameters)
{
    if (!i2c_mutex) {
        ESP_LOGE(TAG, "i2c_mutex not initialized");
        vTaskDelete(NULL);
    }

    while(1) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        get_accelerometer_data();
        xSemaphoreGive(i2c_mutex);
        ESP_LOGI(TAG, "X: %.2f g, Y: %.2f g, Z: %.2f g", x_g, y_g, z_g);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief Initialize the LIS3DH accelerometer.
 *
 * Powers on the device, enables X/Y/Z axes, and sets default data rate.
 */
void accelerometer_sensor_init(void)
{
	// Power ON and enable X/Y/Z axes
	uint8_t write_buf[] = {0x20, 0x57};
	i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
}
