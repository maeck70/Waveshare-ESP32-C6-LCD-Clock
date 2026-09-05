/**
 * @file ws2812.h
 * @brief RMT-based WS2812 addressable RGB LED driver.
 *
 * Drives the onboard addressable RGB LED connected to GPIO 8 on the
 * Waveshare ESP32-C6-LCD-1.47 development board.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPIO pin connected to the onboard WS2812 RGB LED.
 */
#define WS2812_GPIO_PIN 8

/**
 * @brief Initialize RMT peripheral channel and encoder for the WS2812 LED.
 *
 * @return ESP_OK on success, or error code on failure.
 */
esp_err_t ws2812_init(void);

/**
 * @brief Set the color of the onboard WS2812 RGB LED.
 *
 * Transmits 24-bit GRB/RGB timing symbols via RMT peripheral.
 *
 * @param r Red intensity (0..255).
 * @param g Green intensity (0..255).
 * @param b Blue intensity (0..255).
 * @return ESP_OK on success, or error code on failure.
 */
esp_err_t ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
