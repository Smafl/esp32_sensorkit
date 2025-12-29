#pragma once
#include "esp_err.h"

/**
 * @brief Initialize Wi-Fi in STA mode and connect
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_init_sta(void);
