/**
 * @file wifi_time.h
 * @brief Wi-Fi connection and SNTP internet time synchronization for ESP32-C6.
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes NVS, Wi-Fi station, and SNTP service.
 * Connects to the network defined in wifi_config.h (.env).
 * If WIFI_SSID is empty, Wi-Fi is skipped gracefully.
 */
void wifi_time_init(void);

/**
 * @brief Returns true if Wi-Fi is currently connected and has an IP address.
 */
bool wifi_time_is_connected(void);

/**
 * @brief Returns true if system time has been synchronized with the NTP server.
 */
bool wifi_time_is_synced(void);

#ifdef __cplusplus
}
#endif
