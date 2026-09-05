/**
 * @file button.c
 * @brief Implementation of button debounce and monitoring task.
 */

#include "button.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/** Registered callback invoked upon button press */
static button_cb_t s_on_press_cb = NULL;

/**
 * @brief FreeRTOS task that polls and debounces the BOOT button.
 *
 * @param pvParam Unused task parameter.
 */
static void button_task(void *pvParam) {
    (void)pvParam;

    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BOOT_BTN_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_conf);

    bool last_state = true;
    int64_t press_start_time = 0;

    while (1) {
        int state = gpio_get_level(BOOT_BTN_PIN);
        if (last_state && !state) {
            // Transition: idle (high) -> pressed down (low)
            press_start_time = esp_timer_get_time();
        } else if (!last_state && state) {
            // Transition: pressed (low) -> released (high)
            int64_t press_duration_ms = (esp_timer_get_time() - press_start_time) / 1000;
            if (press_duration_ms > 50) {
                if (s_on_press_cb) {
                    s_on_press_cb();
                }
            }
        }
        last_state = state;
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void button_init(button_cb_t on_press) {
    s_on_press_cb = on_press;
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
}
