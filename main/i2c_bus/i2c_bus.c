#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "i2c_bus.h"
#include "sdkconfig.h"
#include "esp_log.h"

static const char *TAG = "i2c_bus";

SemaphoreHandle_t i2c_mutex = NULL;

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */

i2c_master_bus_handle_t i2c_bus = NULL;
i2c_master_dev_handle_t i2c_dev_th_sensor = NULL; // temparature and humidity sensor
i2c_master_dev_handle_t i2c_dev_display = NULL;
i2c_master_dev_handle_t i2c_dev_accelerometer = NULL; // LIS3DH

void i2c_master_init(void)
{
    i2c_mutex = xSemaphoreCreateMutex();
    if( i2c_mutex == NULL )
    {
        ESP_LOGE(TAG, "Failed to create i2c_mutex");
        return;
    }
    ESP_LOGI(TAG, "Created i2c_mutex");

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    // Discover devices
    // for (uint16_t i = 0; i <= 127; i++) {
    //     esp_err_t discover_result = i2c_master_probe(i2c_bus, i, 100);
    //     if (discover_result == ESP_OK) {
    //         ESP_LOGI(TAG, "Found device %hx", i);
    //     }
    //     // } else if (discover_result == ESP_ERR_NOT_FOUND) {
    //     //     ESP_LOGI(TAG, "Device not found");
    //     // } else if (discover_result == ESP_ERR_TIMEOUT) {
    //     //     ESP_LOGI(TAG, "Timeout");
	// }

    // Add temperature and humidity sensor
    i2c_device_config_t dev_config_th_sensor = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x38,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_config_th_sensor, &i2c_dev_th_sensor));

    // Add display
    i2c_device_config_t dev_config_display = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x3c,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 1000,
        .flags = {
            .disable_ack_check = false
        }
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_config_display, &i2c_dev_display));

    // Add accelerometer sensor
    i2c_device_config_t dev_config_accelerometer = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x19,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        .scl_wait_us = 1000,
        .flags = {
            .disable_ack_check = false
        }
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_config_accelerometer, &i2c_dev_accelerometer));
}

// i2c_bus: Found device 19 - accelerometer
// i2c_bus: Found device 38 - th_sensor
// i2c_bus: Found device 3c - display
// i2c_bus: Found device 77 - air pressure
