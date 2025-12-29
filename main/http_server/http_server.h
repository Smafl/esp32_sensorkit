#pragma once
#include "esp_err.h"
#include "esp_http_server.h"
#include "th_sensor.h"

/**
 * @brief Start the HTTP server and register URI handlers
 * @return HTTP server handle, or NULL on failure
 */
httpd_handle_t start_webserver(void);
