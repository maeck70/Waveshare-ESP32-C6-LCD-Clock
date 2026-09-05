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
 * @brief RGB color struct (8-bit per channel).
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

/**
 * @brief Initialize the 256-color warm rainbow glow palette (orange to red).
 */
void effects_init(void);

/**
 * @brief Get precomputed RGB color from the glow palette by 8-bit index.
 *
 * @param index 8-bit cyclical index (0..255).
 * @param r Output red channel.
 * @param g Output green channel.
 * @param b Output blue channel.
 */
void effects_get_palette_color(uint8_t index, uint8_t *r, uint8_t *g, uint8_t *b);

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
 * @brief Update glowing color animation cycle for WS2812 underglow LED and global phase.
 *
 * @param[in,out] phase Pointer to animation phase angle in radians (updated on each call).
 * @param[out]    led_r Output red value for onboard WS2812 RGB LED.
 * @param[out]    led_g Output green value for onboard WS2812 RGB LED.
 * @param[out]    led_b Output blue value for onboard WS2812 RGB LED.
 */
void effects_update(float *phase, uint8_t *led_r, uint8_t *led_g, uint8_t *led_b);

#ifdef __cplusplus
}
#endif
