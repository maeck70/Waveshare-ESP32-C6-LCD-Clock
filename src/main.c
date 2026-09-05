/**
 * @file main.c
 * @brief Application entry point for Waveshare ESP32-C6-LCD-1.47 Clock & HUD.
 *
 * Orchestrates clock timekeeping, display rendering, user button interactions,
 * and WS2812 ambient lighting animations.
 */

#include <stdint.h>
#include "button.h"
#include "clock.h"
#include "effects.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui.h"
#include "wifi_time.h"
#include "ws2812.h"

static const char *TAG = "MAIN_APP";

/**
 * @brief Callback triggered when the tactile BOOT button (GPIO 9) is pressed.
 *
 * Toggles the ST7789 display orientation between standard landscape (0)
 * and 180-degree inverted landscape (1).
 */
static void on_boot_button_pressed(void) {
    ui_toggle_rotation();
    ESP_LOGI(TAG, "Orientation toggled: %d", ui_get_rotation());
}

/**
 * @brief Main application entry point.
 */
void app_main(void) {
    ESP_LOGI(TAG, "Starting Waveshare ESP32-C6-LCD-1.47 Clock App");

    // Initialize clock state from compilation time (__TIME__) & default date
    clock_init();

    // Initialize ST7789 display controller and UI engine (default landscape)
    ui_init(0);

    // Initialize onboard WS2812 RGB LED (back glow underglow light)
    ws2812_init();

    // Start background tasks for BOOT button input and serial time sync
    button_init(on_boot_button_pressed);
    clock_serial_sync_init();

    // Initialize Wi-Fi connection and SNTP internet time sync (configured in .env)
    wifi_time_init();

    ESP_LOGI(TAG, "Clock and rainbow glow animation running...");

    int64_t last_log_time = esp_timer_get_time();
    float phase = 0.0f;
    uint8_t led_r, led_g, led_b;
    char time_str[16];

    while (1) {
        int64_t now = esp_timer_get_time();

        // Advance 1-second clock counters
        clock_tick();

        // Print status telemetry to serial monitor every 5 seconds
        if (now - last_log_time >= 5000000) {
            last_log_time = now;
            clock_get_time_str(time_str, sizeof(time_str));
            ESP_LOGI(TAG, "Tick: %s | Date: %s", time_str, clock_get_date_str());
        }

        // Update glow animation phase and LED underglow color
        effects_update(&phase, &led_r, &led_g, &led_b);

        // Update the onboard WS2812 underglow RGB LED
        ws2812_set_rgb(led_r, led_g, led_b);

        // Render current scene with 75-degree angled glow wave
        clock_get_time_str(time_str, sizeof(time_str));
        ui_render_scene(time_str, phase, clock_get_date_str());

        // 35ms frame delay (~28 FPS)
        vTaskDelay(pdMS_TO_TICKS(35));
    }
}
