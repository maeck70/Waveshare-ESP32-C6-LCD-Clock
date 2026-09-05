/**
 * @file ui.h
 * @brief User interface and scene rendering engine.
 *
 * Coordinates screen layout, anti-aliased font rasterization, display
 * orientation, and scene composition for the Waveshare ST7789 display.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the UI engine and ST7789 display controller.
 *
 * @param initial_rotation Initial orientation (0: landscape, 1: 180-deg flipped landscape).
 */
void ui_init(uint8_t initial_rotation);

/**
 * @brief Set display rotation.
 *
 * @param rotation Rotation value (0: standard landscape, 1: 180-deg flipped landscape).
 */
void ui_set_rotation(uint8_t rotation);

/**
 * @brief Retrieve current display rotation.
 *
 * @return Current rotation value (0 or 1).
 */
uint8_t ui_get_rotation(void);

/**
 * @brief Toggle display rotation between standard landscape (0) and 180-degree flip (1).
 */
void ui_toggle_rotation(void);

/**
 * @brief Render the complete scene (time digits with 75-degree glowing wave, date string) and flush to display.
 *
 * @param time_str Null-terminated string formatted as "HH:MM:SS" (8 characters).
 * @param phase Current glowing wave animation phase in radians.
 * @param date_str Null-terminated date string to display in the bottom row.
 */
void ui_render_scene(const char *time_str, float phase, const char *date_str);

#ifdef __cplusplus
}
#endif
