#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "accelerometer.h"
#include "i2c_bus/i2c_bus.h"
#include "esp_log.h"

static const char *TAG = "accelerometer";

#define LIS3DH_REG_CTRL1            0x20
#define LIS3DH_REG_CTRL2            0x21
#define LIS3DH_REG_CTRL4            0x23
#define LIS3DH_REG_OUT_X_L          0x28

#define LIS3DY_HPCLICK              (1 << 2)
#define LIS3DH_CLICK_CFG            0x38
#define LIS3DH_CLICK_THS            0x3a
#define LIS3DH_CLICK_THS_LIR_CLICK  (1 << 7)
#define LIS3DH_CLICK_SRC            0x39

#define LIS3DH_CLICK_CFG_XS         (1 << 0)
#define LIS3DH_CLICK_CFG_XD         (1 << 1)
#define LIS3DH_CLICK_CFG_YS         (1 << 2)
#define LIS3DH_CLICK_CFG_YD         (1 << 3)
#define LIS3DH_CLICK_CFG_ZS         (1 << 4)
#define LIS3DH_CLICK_CFG_ZD         (1 << 5)

#define CLICK_IA                    (1 << 6)
#define CLICK_DCLICK                (1 << 5)
#define CLICK_SCLICK                (1 << 4)

#define LIS3DH_X_ENABLE             (1 << 0)
#define LIS3DH_Y_ENABLE             (1 << 1)
#define LIS3DH_Z_ENABLE             (1 << 2)

#define LIS3DH_ODR0                 (1 << 4)
#define LIS3DH_ODR1                 (1 << 5)
#define LIS3DH_ODR2                 (1 << 6)
#define LIS3DH_ODR3                 (1 << 7)
#define LIS3DH_ODR_100HZ            (LIS3DH_ODR2 | LIS3DH_ODR0)

#define LIS3DH_TIME_LIMIT           0x3b
#define LIS3DH_TIME_LATENCY         0x3c
#define LIS3DH_TIME_WINDOW          0x3d

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

    /*
    Reads the CTRL4 register of the LIS3DH. This register stores:
    - The full-scale range (±2g, ±4g, ±8g, ±16g)
    - Whether high-resolution mode is enabled.
    */
    if (lis3dh_read(LIS3DH_REG_CTRL4, &ctrl4, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CTRL_REG4");
        return;
    }

    uint8_t range_bits = (ctrl4 >> 4) & 0x03; // Determine measurement range
    uint8_t hr_bit = (ctrl4 >> 3) & 0x01; // Bit 3 of CTRL4, indicates high-resolution mode

    // Maps the 2-bit range_bits to the actual physical acceleration range in g
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

    // Combines low and high bytes into a signed 16-bit integer
    int16_t x_raw = (int16_t)(raw[0] | (raw[1] << 8));
    int16_t y_raw = (int16_t)(raw[2] | (raw[3] << 8));
    int16_t z_raw = (int16_t)(raw[4] | (raw[5] << 8));

    // ESP_LOGI(TAG, "Z: %hd g, range = %hhu, hr = %hhu", z_raw, range_bits, hr_bit);

    // Converts raw integer values to acceleration in g
    if (bits == 12) {
        x_g = (float)x_raw * range_g / 32000.0f / 4.0f;
        y_g = (float)y_raw * range_g / 32000.0f / 4.0f;
        z_g = (float)z_raw * range_g / 32000.0f / 4.0f;
    } else {
        x_g = (float)x_raw * range_g / 32000.0f;
        y_g = (float)y_raw * range_g / 32000.0f;
        z_g = (float)z_raw * range_g / 32000.0f;
    }

    uint8_t click_src;
    if (lis3dh_read(LIS3DH_CLICK_SRC, &click_src, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read CLICK_SRC");
        return;
    }
    // ESP_LOGI(TAG, "REG %02hhx", click_src);

    if (click_src & CLICK_DCLICK) {
        ESP_LOGI(TAG, "DOUBLE click");
    }
    // } else if (click_src & CLICK_SCLICK) {
    //     ESP_LOGI(TAG, "SINGLE click");
    // }
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
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Initialize the LIS3DH accelerometer.
 *
 * Powers on the device and set up configurations
 */
void accelerometer_sensor_init(void)
{
	{
        // Power ON and enable X/Y/Z axes
        uint8_t write_buf[] = {LIS3DH_REG_CTRL1, LIS3DH_ODR_100HZ | LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE};
        i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }

	{
        // High-pass filter enabled for CLICK function
        uint8_t write_buf[] = {LIS3DH_REG_CTRL2, LIS3DY_HPCLICK};
        i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }

    // {
    // Enable HPCLICK with safe read-modify-write
    // uint8_t reg2;
    // lis3dh_read(LIS3DH_REG_CTRL2, &reg2, 1);
    // reg2 |= LIS3DY_HPCLICK;         // bit 2
    // reg2 &= ~((1 << 7) | (1 << 6)); // HPM1:HPM0 = 00
    // i2c_master_transmit(i2c_dev_accelerometer, &reg2, 1, 50);
    // }

    {
        // Double click
        uint8_t write_buf[] = {LIS3DH_CLICK_CFG, LIS3DH_CLICK_CFG_ZD};
	    i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }

    // {
    //     // Single and double click
    //     uint8_t write_buf[] = {LIS3DH_CLICK_CFG,
    //         LIS3DH_CLICK_CFG_XD | LIS3DH_CLICK_CFG_YD | LIS3DH_CLICK_CFG_ZD |
    //         LIS3DH_CLICK_CFG_XS | LIS3DH_CLICK_CFG_YS | LIS3DH_CLICK_CFG_ZS};
    //     i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    // }

    {
        // Click threshold
        uint8_t write_buf[] = {LIS3DH_CLICK_THS, 20 | LIS3DH_CLICK_THS_LIR_CLICK};
	    i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }

    {
        // Time limit
        uint8_t write_buf[] = {LIS3DH_TIME_LIMIT, 10};
	    i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }

    {
        // Time latency
        uint8_t write_buf[] = {LIS3DH_TIME_LATENCY, 20};
	    i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }

    {
        // Time window
        uint8_t write_buf[] = {LIS3DH_TIME_WINDOW, 40};
	    i2c_master_transmit(i2c_dev_accelerometer, write_buf, 2, 50);
    }
}
