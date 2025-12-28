set(requires esp-tls nvs_flash esp_netif esp_http_server esp_driver_i2c u8g2)
idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    list(APPEND requires esp_stubs protocol_examples_common)
else()
    list(APPEND requires esp_wifi esp_eth)
endif()

idf_component_register(
    SRCS "main.c"
         "http_server/http_server.c"
         "wifi_manager/wifi_manager.c"
    INCLUDE_DIRS "." "http_server" "wifi_manager"
    PRIV_REQUIRES ${requires}
)
