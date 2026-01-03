set(requires esp-tls nvs_flash esp_netif esp_http_server esp_driver_i2c u8g2 esp_http_client)
idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    list(APPEND requires esp_stubs protocol_examples_common)
else()
    list(APPEND requires esp_wifi esp_eth)
endif()

idf_component_register(
    SRCS "main.c"
         "wifi_manager/wifi_manager.c"
         "http_server/http_server.c"
         "i2c_bus/i2c_bus.c"
         "display/display.c"
         "th_sensor/th_sensor.c"
         "accelerometer/accelerometer.c"
         "tasks/tasks.c"
    INCLUDE_DIRS "."
                 "wifi_manager"
                 "http_server"
                 "i2c_bus"
                 "display"
                 "th_sensor"
                 "accelerometer"
                 "tasks"
    PRIV_REQUIRES ${requires} json
)
