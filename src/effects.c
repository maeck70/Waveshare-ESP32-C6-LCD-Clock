/**
 * @file effects.c
 * @brief Implementation of HSV color conversion and dynamic glow calculations.
 */

#include "effects.h"

#include <math.h>
#include <stdint.h>

/** 2 * PI constant for cyclical phase wrap-around */
#define TWO_PI 6.283185307179586f

void effects_hsv_to_rgb(int h, int s, int v, uint8_t *r_out, uint8_t *g_out, uint8_t *b_out) {
    if (s == 0) {
        *r_out = *g_out = *b_out = (uint8_t)v;
        return;
    }
    h = h % 360;
    if (h < 0) h += 360;
    int region = h / 60;
    int remainder = (h - (region * 60)) * 6;
    int p = (v * (255 - s)) >> 8;
    int q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    int t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    int r, g, b;
    switch (region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    *r_out = (uint8_t)r;
    *g_out = (uint8_t)g;
    *b_out = (uint8_t)b;
}

static rgb_color_t s_palette[256];
static bool s_palette_init = false;

void effects_init(void) {
    if (s_palette_init) return;
    for (int i = 0; i < 256; i++) {
        float theta = (float)i * (float)TWO_PI / 256.0f;
        // Continuous, perfectly smooth cosine rainbow from Dark Red -> Orange -> Yellowish Orange -> Orange -> Dark Red:
        // Dark Red is (204, 0, 0) (red with 20% black / 80% red)
        // Yellowish Orange peak is (255, 185, 0)
        // R smoothly transitions from 204 (Dark Red) to 255 (Yellowish Orange) and back
        // G smoothly transitions from 0 (Dark Red) to 185 (Yellowish Orange) and back
        // B is always 0
        float blend = 0.5f * (1.0f - cosf(theta)); // 0.0 at Dark Red, 1.0 at Yellowish Orange
        float r_f = 204.0f + 51.0f * blend + 0.5f;
        int r = (int)r_f;
        if (r < 204) r = 204;
        if (r > 255) r = 255;

        float g_f = 185.0f * blend + 0.5f;
        int g = (int)g_f;
        if (g < 0) g = 0;
        if (g > 185) g = 185;

        s_palette[i].r = (uint8_t)r;
        s_palette[i].g = (uint8_t)g;
        s_palette[i].b = 0;
    }
    s_palette_init = true;
}

void effects_get_palette_color(uint8_t index, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (!s_palette_init) {
        effects_init();
    }
    if (r) *r = s_palette[index].r;
    if (g) *g = s_palette[index].g;
    if (b) *b = s_palette[index].b;
}

void effects_update(float *phase, uint8_t *led_r, uint8_t *led_g, uint8_t *led_b) {
    if (!phase) return;
    if (!s_palette_init) {
        effects_init();
    }

    // Smooth and slow animation phase (~18-second full sweep cycle at ~28 FPS)
    *phase += 0.012f;
    if (*phase > TWO_PI) {
        *phase -= TWO_PI;
    }

    float p = *phase;

    // Compute synchronized underglow light
    if (led_r && led_g && led_b) {
        uint8_t idx = (uint8_t)((int)(p * (256.0f / (float)TWO_PI)) & 0xFF);
        effects_get_palette_color(idx, led_r, led_g, led_b);
    }
}
