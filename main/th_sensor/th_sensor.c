#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "th_sensor.h"
#include "i2c_bus/i2c_bus.h"
#include "display/display.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "sdkconfig.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define SERVER_URL "http://" CONFIG_SERVER_IP ":" STR(CONFIG_SERVER_PORT) "/th_sensor"

static const char *TAG = "th_sensor";

float temperature = 0;
float humidity = 0;

/**
 * @brief Send the latest temperature and humidity data to a server.
 *
 * Creates a JSON payload containing `temperature`, `humidity`, and `timestamp`,
 * then performs an HTTP POST request to the configured server URL.
 * Logs success or error messages accordingly.
 */
void send_th_sensor_data(void)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "temperature", temperature);
    cJSON_AddNumberToObject(json, "humidity", humidity);
    cJSON_AddNumberToObject(json, "timestamp", (uint32_t)time(NULL));

    char *payload = cJSON_PrintUnformatted(json);

    /**
     * NOTE: All the configuration parameters for http_client must be specified either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered.
     */
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
    };
    ESP_LOGI(TAG, "HTTP request with url => %s", SERVER_URL);
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        if (status == 200 || status == 201) {
            ESP_LOGI(TAG, "Data accepted by server");
        } else {
            ESP_LOGW(TAG, "Server responded with %d", status);
        }
    } else {
        ESP_LOGE(TAG, "HTTP error: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    cJSON_Delete(json);
    free(payload);
}

/**
 * @brief Read temperature and humidity data from the sensor over I2C.
 *
 * This function sends the measurement command to the sensor,
 * reads raw data, converts it to physical values, and stores them
 * in the static variables `temperature` and `humidity`.
 */
void get_th_sensor_data(void)
{
    // 0xAC → trigger measurement; 0x33 → command parameter; 0x00 → command parameter
    uint8_t write_buf[] = {0xac, 0x33, 0x00};
    uint8_t read_buf[6];
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_dev_th_sensor, write_buf, 3, 50));
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_ERROR_CHECK(i2c_master_receive(i2c_dev_th_sensor, read_buf, 6, 50));

    uint32_t hum_raw = (read_buf[1] << 16 | read_buf[2] << 8 | read_buf[3]) >> 4;
    uint32_t temp_raw = (read_buf[3] << 16 | read_buf[4] << 8 | read_buf[5]) & 0xfffff;

    temperature = temp_raw * 200.0 / (1024*1024) - 50;
    humidity = hum_raw * 100.0 / (1024*1024);

    ESP_LOGI(TAG, "Temp: %.1f; Humid: %.1f", temperature, humidity);
}

/**
 * @brief FreeRTOS task that periodically reads sensor data, updates the display,
 *        and sends the data to the server.
 *
 * The task loops indefinitely with a 2-second delay between iterations.
 *
 * @param pvParameters Not used.
 */
void th_sensor_update_task(void *pvParameters)
{
    if (!i2c_mutex) {
        ESP_LOGE(TAG, "i2c_mutex not initialized");
        vTaskDelete(NULL);
    }

    while(1) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        get_th_sensor_data();
        display_th_sensor_data(temperature, humidity);
        xSemaphoreGive(i2c_mutex);
        send_th_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
