#include "http_server.h"
#include "esp_log.h"

#define TH_BUF_SIZE 256

static const char *TAG = "http_server";

// favicon
static esp_err_t favicon_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, "", 0);
    return ESP_OK;
}

static const httpd_uri_t favicon_uri = {
    .uri = "/favicon.ico",
    .method = HTTP_GET,
    .handler = favicon_get_handler
};

// hello
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, "Hello ESP32", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri      = "/hello",
    .method   = HTTP_GET,
    .handler  = hello_get_handler,
};

// th_sensor
static esp_err_t th_sensor_post_handler(httpd_req_t *req)
{
    char buf[TH_BUF_SIZE];
    int ret = httpd_req_recv(req, buf, TH_BUF_SIZE);
    if (ret > 0) {
        buf[ret] = '\0';  // null-terminate
        ESP_LOGI("TH_SENSOR", "Received: %s", buf);
    }

    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t th_sensor = {
    .uri      = "/th_sensor",
    .method   = HTTP_POST,
    .handler  = th_sensor_post_handler,
};

// server
httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 8000;

    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &th_sensor);
        httpd_register_uri_handler(server, &favicon_uri);
        return server;
    }

    ESP_LOGE(TAG, "Error starting server!");
    return NULL;
}
