/**
 * @file button.h
 * @brief Tactile button input driver for Waveshare ESP32-C6-LCD-1.47.
 *
 * Provides active-low tactile button monitoring with debounce handling
 * and press callback dispatching.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPIO pin connected to the onboard BOOT button.
 */
#define BOOT_BTN_PIN 9

/**
 * @brief Callback function prototype invoked upon valid button press.
 */
typedef void (*button_cb_t)(void);

/**
 * @brief Initialize the BOOT button input monitoring task.
 *
 * Configures the GPIO pin as input with internal pull-up enabled and
 * spawns a background FreeRTOS task to sample and debounce button events.
 *
 * @param on_press Callback invoked when a button press (>50ms) is released.
 */
void button_init(button_cb_t on_press);

#ifdef __cplusplus
}
#endif
