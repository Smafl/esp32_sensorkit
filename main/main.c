
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "wifi_manager/wifi_manager.h"
#include "http_server/http_server.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    wifi_init_sta();

    ESP_LOGI(TAG, "Starting HTTP server...");
    httpd_handle_t server = start_webserver();

    while (server) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
