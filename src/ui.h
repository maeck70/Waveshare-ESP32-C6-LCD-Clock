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
 * @brief Render the complete scene (time digits, separator accent, date string) and flush to display.
 *
 * @param time_str Null-terminated string formatted as "HH:MM:SS" (8 characters).
 * @param time_r Array of 8 red channel color values for each digit/colon character.
 * @param time_g Array of 8 green channel color values for each digit/colon character.
 * @param time_b Array of 8 blue channel color values for each digit/colon character.
 * @param date_str Null-terminated date string to display in the bottom row.
 */
void ui_render_scene(const char *time_str,
                     const uint8_t time_r[8],
                     const uint8_t time_g[8],
                     const uint8_t time_b[8],
                     const char *date_str);

#ifdef __cplusplus
}
#endif
