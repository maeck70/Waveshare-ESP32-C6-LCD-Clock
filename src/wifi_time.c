/**
 * @file wifi_time.c
 * @brief Wi-Fi connection and SNTP internet time synchronization for ESP32-C6.
 */

#include "wifi_time.h"
#include "wifi_config.h"
#include "clock.h"

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "WIFI_TIME";

static bool s_is_connected = false;
static bool s_is_synced = false;
static bool s_sntp_initialized = false;

bool wifi_time_is_connected(void) {
    return s_is_connected;
}

bool wifi_time_is_synced(void) {
    return s_is_synced;
}

static void time_sync_notification_cb(struct timeval *tv) {
    s_is_synced = true;
    time_t now = tv->tv_sec;
    struct tm ti;
    localtime_r(&now, &ti);

    ESP_LOGI(TAG, "NTP Time synchronized successfully: %02d:%02d:%02d (%04d-%02d-%02d)",
             ti.tm_hour, ti.tm_min, ti.tm_sec,
             1900 + ti.tm_year, ti.tm_mon + 1, ti.tm_mday);

    clock_set_time(ti.tm_hour, ti.tm_min, ti.tm_sec);

    static const char *const months[12] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    if (ti.tm_mon >= 0 && ti.tm_mon < 12) {
        char date_buf[32];
        snprintf(date_buf, sizeof(date_buf), "%s %d, %d",
                 months[ti.tm_mon], ti.tm_mday, 1900 + ti.tm_year);
        clock_set_date(date_buf);
    }
}

static void init_sntp(void) {
    if (s_sntp_initialized) return;

    ESP_LOGI(TAG, "Initializing SNTP with server: %s, TZ: %s", NTP_SERVER_STR, TIMEZONE_STR);

    // Set local timezone for POSIX functions (e.g. localtime_r)
    setenv("TZ", TIMEZONE_STR, 1);
    tzset();

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER_STR);
    config.sync_cb = time_sync_notification_cb;

    esp_err_t err = esp_netif_sntp_init(&config);
    if (err == ESP_OK) {
        s_sntp_initialized = true;
        ESP_LOGI(TAG, "SNTP service started.");
    } else {
        ESP_LOGE(TAG, "Failed to initialize SNTP: %s", esp_err_to_name(err));
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi station started, connecting to '%s'...", WIFI_SSID);
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_is_connected = false;
        ESP_LOGW(TAG, "Wi-Fi disconnected. Reconnecting in 3 seconds...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        s_is_connected = true;
        ESP_LOGI(TAG, "Wi-Fi connected! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        init_sntp();
    }
}

void wifi_time_init(void) {
    // 1. Initialize NVS (required for Wi-Fi stack)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Set timezone environment variable immediately
    setenv("TZ", TIMEZONE_STR, 1);
    tzset();

    // Check if Wi-Fi credentials are provided
    if (strlen(WIFI_SSID) == 0) {
        ESP_LOGW(TAG, "No WIFI_SSID configured in .env. Skipping Wi-Fi time sync.");
        return;
    }

    // 3. Initialize TCP/IP network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 4. Initialize Wi-Fi Driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register Wi-Fi & IP event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // 5. Configure Wi-Fi station mode with credentials from .env
    wifi_config_t wifi_config = { 0 };
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization complete for SSID: '%s'", WIFI_SSID);
}
