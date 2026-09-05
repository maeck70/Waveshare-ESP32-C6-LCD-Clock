/**
 * @file clock.c
 * @brief Implementation of timekeeping and serial sync tasks.
 */

#include "clock.h"
#include "wifi_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CLOCK";

/** Current clock time state */
static int s_hours = 0;
static int s_minutes = 0;
static int s_seconds = 0;

/** Current date string state */
static char s_date_str[32] = "September 3, 2026";

/** Timestamp in microseconds of the last full second tick */
static int64_t s_last_sec_time = 0;

void clock_init(void) {
    // Initialize time from compilation time (__TIME__ format: "HH:MM:SS")
    s_hours   = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
    s_minutes = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
    s_seconds = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');

    s_last_sec_time = esp_timer_get_time();

    ESP_LOGI(TAG, "Clock initialized to %02d:%02d:%02d, Date: %s",
             s_hours, s_minutes, s_seconds, s_date_str);
}

void clock_tick(void) {
    if (wifi_time_is_synced()) {
        time_t now;
        struct tm ti;
        time(&now);
        localtime_r(&now, &ti);
        s_hours   = ti.tm_hour;
        s_minutes = ti.tm_min;
        s_seconds = ti.tm_sec;

        static const char *const months[12] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };
        if (ti.tm_mon >= 0 && ti.tm_mon < 12) {
            snprintf(s_date_str, sizeof(s_date_str), "%s %d, %d",
                     months[ti.tm_mon], ti.tm_mday, 1900 + ti.tm_year);
        }
        return;
    }

    int64_t now = esp_timer_get_time();
    if (now - s_last_sec_time >= 1000000) {
        int elapsed_secs = (int)((now - s_last_sec_time) / 1000000);
        s_last_sec_time += (int64_t)elapsed_secs * 1000000;

        s_seconds += elapsed_secs;
        while (s_seconds >= 60) {
            s_seconds -= 60;
            s_minutes++;
            if (s_minutes >= 60) {
                s_minutes = 0;
                s_hours++;
                if (s_hours >= 24) {
                    s_hours = 0;
                }
            }
        }
    }
}

void clock_get_time(int *hours, int *minutes, int *seconds) {
    if (hours)   *hours   = s_hours;
    if (minutes) *minutes = s_minutes;
    if (seconds) *seconds = s_seconds;
}

void clock_get_time_str(char *buf, size_t max_len) {
    if (!buf || max_len == 0) return;
    snprintf(buf, max_len, "%02d:%02d:%02d", s_hours, s_minutes, s_seconds);
}

const char *clock_get_date_str(void) {
    return s_date_str;
}

void clock_set_time(int hours, int minutes, int seconds) {
    s_hours = hours % 24;
    s_minutes = minutes % 60;
    s_seconds = seconds % 60;
    s_last_sec_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Time set to: %02d:%02d:%02d", s_hours, s_minutes, s_seconds);
}

void clock_set_date(const char *date_str) {
    if (!date_str) return;
    strncpy(s_date_str, date_str, sizeof(s_date_str) - 1);
    s_date_str[sizeof(s_date_str) - 1] = '\0';
    ESP_LOGI(TAG, "Date set to: %s", s_date_str);
}

/**
 * @brief FreeRTOS task reading incoming ASCII time/date packets over USB serial.
 *
 * @param pvParam Unused task parameter.
 */
static void serial_sync_task(void *pvParam) {
    (void)pvParam;
    char buf[64];
    int idx = 0;

    while (1) {
        int c = getchar();
        if (c != EOF && c > 0) {
            if (c == '\n' || c == '\r') {
                if (idx > 0) {
                    buf[idx] = '\0';
                    int h, m, s;
                    if (sscanf(buf, "%d:%d:%d", &h, &m, &s) == 3) {
                        clock_set_time(h, m, s);
                    } else if (strncmp(buf, "DATE=", 5) == 0) {
                        clock_set_date(buf + 5);
                    }
                    idx = 0;
                }
            } else if (idx < (int)(sizeof(buf) - 1)) {
                buf[idx++] = (char)c;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void clock_serial_sync_init(void) {
    xTaskCreate(serial_sync_task, "serial_sync_task", 2048, NULL, 4, NULL);
}
