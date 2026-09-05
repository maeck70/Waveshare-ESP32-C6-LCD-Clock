/**
 * @file effects.h
 * @brief Color math and dynamic ambient LED/HUD lighting effects.
 *
 * Provides HSV-to-RGB conversion and animated warm rainbow color calculation
 * for clock digits and synchronized underglow WS2812 RGB LED.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert HSV color coordinates to 24-bit standard RGB.
 *
 * @param[in]  h Hue angle in degrees (0..360, automatically wrapped).
 * @param[in]  s Saturation (0..255).
 * @param[in]  v Value / brightness (0..255).
 * @param[out] r_out Output red component (0..255).
 * @param[out] g_out Output green component (0..255).
 * @param[out] b_out Output blue component (0..255).
 */
void effects_hsv_to_rgb(int h, int s, int v, uint8_t *r_out, uint8_t *g_out, uint8_t *b_out);

/**
 * @brief Update glowing color animation cycle for clock characters and WS2812 LED.
 *
 * Advances animation phase and calculates the 8-character warm gradient
 * across "HH:MM:SS" alongside the underglow LED.
 *
 * @param[in,out] phase Pointer to animation phase angle in radians (updated on each call).
 * @param[out]    time_r Output red channel array for 8 time characters.
 * @param[out]    time_g Output green channel array for 8 time characters.
 * @param[out]    time_b Output blue channel array for 8 time characters.
 * @param[out]    led_r Output red value for onboard WS2812 RGB LED.
 * @param[out]    led_g Output green value for onboard WS2812 RGB LED.
 * @param[out]    led_b Output blue value for onboard WS2812 RGB LED.
 */
void effects_update(float *phase,
                    uint8_t time_r[8], uint8_t time_g[8], uint8_t time_b[8],
                    uint8_t *led_r, uint8_t *led_g, uint8_t *led_b);

#ifdef __cplusplus
}
#endif
