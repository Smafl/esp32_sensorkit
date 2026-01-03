#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "th_sensor/th_sensor.h"
#include "accelerometer/accelerometer.h"

// static const char *TAG = "tasks";

/**
 * @brief Create and start the FreeRTOS task that handles sensor updates.
 *
 * This function wraps the task creation and sets the stack size and priority.
 */
void th_sensor_start_task(void)
{
    xTaskCreate(th_sensor_update_task,
                "th_sensor_update",
                8192,
                NULL,
                tskIDLE_PRIORITY,
                NULL
                );
}

void accelerometer_start_task(void)
{
    xTaskCreate(accelerometer_update_task,
                "accelerometer_update",
                8192,
                NULL,
                tskIDLE_PRIORITY,
                NULL
                );
}
