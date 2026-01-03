#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"

#include "wifi_manager/wifi_manager.h"
#include "http_server/http_server.h"
#include "i2c_bus/i2c_bus.h"
#include "display/display.h"
#include "th_sensor/th_sensor.h"
#include "accelerometer/accelerometer.h"
#include "tasks/tasks.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing NVS...");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Connect to Wi-Fi
    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    wifi_init_sta();

    // Start HTTP server
    ESP_LOGI(TAG, "Starting HTTP server...");
    httpd_handle_t server = start_webserver();
    if (!server) {
        ESP_LOGE(TAG, "Failed to start HTTP server!");
    }

    // Initialize I2C bus and devices
    ESP_LOGI(TAG, "Initializing I2C bus...");
    i2c_master_init();

    // Initialize OLED display
    ESP_LOGI(TAG, "Initializing display...");
    u8g2_display_init();

    // Start accelerometer sensor task
    ESP_LOGI(TAG, "Get accelerometer data...");
    accelerometer_sensor_init();
    accelerometer_start_task();

    // Start temperature & humidity sensor task
    ESP_LOGI(TAG, "Starting TH sensor task...");
    th_sensor_start_task();

    // Keep main alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
