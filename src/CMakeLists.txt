idf_component_register(SRCS "main.c" "st7789.c" "ws2812.c" "clock.c" "button.c" "effects.c" "ui.c" "wifi_time.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_rmt driver esp_wifi esp_netif nvs_flash esp_event)

