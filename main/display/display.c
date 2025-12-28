#include "display.h"
#include "i2c_bus/i2c_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// static const char *TAG = "display";

u8g2_t u8g2; // a structure which will contain all the data for one display

static uint8_t u8x8_byte_esp32_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    uint8_t *data;

    switch(msg) {
        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *)arg_ptr;
            while(arg_int--) buffer[buf_idx++] = *data++;
            break;
        case U8X8_MSG_BYTE_INIT:
            break;
        case U8X8_MSG_BYTE_SET_DC:
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            i2c_master_transmit(i2c_dev_display, buffer, buf_idx, 1000 / portTICK_PERIOD_MS);
            break;
        default:
            return 0;
    }
    return 1;
}

// GPIO and delay function for u8g2
static uint8_t u8x8_gpio_and_delay_esp32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT: break;
        case U8X8_MSG_DELAY_MILLI: vTaskDelay(pdMS_TO_TICKS(arg_int)); break;
        case U8X8_MSG_DELAY_10MICRO: vTaskDelay(pdMS_TO_TICKS(0.01 * arg_int)); break;
        case U8X8_MSG_DELAY_100NANO: vTaskDelay(pdMS_TO_TICKS(0.0001)); break;
        default: return 0;
    }
    return 1;
}

void u8g2_display_init(void) {
    u8g2_Setup_ssd1306_i2c_128x32_univision_f(&u8g2, U8G2_R0, u8x8_byte_esp32_i2c, u8x8_gpio_and_delay_esp32);
    u8g2_InitDisplay(&u8g2);
    vTaskDelay(pdMS_TO_TICKS(100));  // Add a 100ms delay
    u8g2_SetPowerSave(&u8g2, 0);  // Wake up display
    u8g2_ClearBuffer(&u8g2);      // Clear the internal buffer
    u8g2_SetFont(&u8g2, u8g2_font_8x13B_tr);  // Set font
    u8g2_DrawStr(&u8g2, 0, 15, "Hello World!"); // Draw text
    u8g2_SendBuffer(&u8g2);       // Transfer buffer to display
}

void display_th_sensor_data(float temperature, float humidity) {
    char buf[16];
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_8x13B_tr);

    snprintf(buf, sizeof(buf), "T %.1f", temperature);
    u8g2_DrawStr(&u8g2, 0, 15, buf);

    snprintf(buf, sizeof(buf), "H %.1f", humidity);
    u8g2_DrawStr(&u8g2, 0, 30, buf);

    u8g2_SendBuffer(&u8g2);
}
