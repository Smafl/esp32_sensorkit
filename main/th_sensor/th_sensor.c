#include "th_sensor.h"
#include "i2c_bus/i2c_bus.h"
#include "display/display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "th_sensor";

static float temperature = 0;
static float humidity = 0;

static void get_th_sensor_data(void) {
    uint8_t write_buf[] = {0xac, 0x33, 0x00};
    uint8_t read_buf[6];
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_dev_th_sensor, write_buf, 3, 50));
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_ERROR_CHECK(i2c_master_receive(i2c_dev_th_sensor, read_buf, 6, 50));

    uint32_t hum = (read_buf[1] << 16 | read_buf[2] << 8 | read_buf[3]) >> 4;
    uint32_t temp = (read_buf[3] << 16 | read_buf[4] << 8 | read_buf[5]) & 0xfffff;

    temperature = temp * 200.0 / (1024*1024) - 50;
    humidity = hum * 100.0 / (1024*1024);

    ESP_LOGI(TAG, "Temp: %.2f; Humid: %.2f", temperature, humidity);
}

static void th_sensor_update_task(void *pvParameters) {
    while(1) {
        get_th_sensor_data();
        display_th_sensor_data(temperature, humidity);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void th_sensor_start_task(void) {
    BaseType_t xReturned;
    xReturned = xTaskCreate(th_sensor_update_task, "th_sensor_update", 2048, NULL, tskIDLE_PRIORITY, NULL);
    (void)xReturned;
}
