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

void effects_update(float *phase,
                    uint8_t time_r[8], uint8_t time_g[8], uint8_t time_b[8],
                    uint8_t *led_r, uint8_t *led_g, uint8_t *led_b) {
    if (!phase) return;

    // Advance glow animation phase slowly (~6.3-second full cycle)
    *phase += 0.035f;
    if (*phase > TWO_PI) {
        *phase -= TWO_PI;
    }

    float p = *phase;

    // Compute warm rainbow glow across the 8 characters of "HH:MM:SS" from Orange to Red
    if (time_r && time_g && time_b) {
        for (int i = 0; i < 8; i++) {
            float h_f = 17.5f + 17.0f * sinf(p + (float)i * 0.44f);
            int h = (int)h_f;
            if (h < 0) h = 0;
            if (h > 40) h = 40;

            int v = 225 + (int)(30.0f * sinf(p * 1.5f + (float)i * 0.25f));
            if (v > 255) v = 255;
            if (v < 180) v = 180;

            int s = 245 + (int)(10.0f * cosf(p + (float)i * 0.2f));
            if (s > 255) s = 255;
            if (s < 220) s = 220;

            effects_hsv_to_rgb(h, s, v, &time_r[i], &time_g[i], &time_b[i]);
        }
    }

    // Compute synchronized orange-to-red glow for the WS2812 back glow light
    if (led_r && led_g && led_b) {
        float center_h = 17.5f + 17.0f * sinf(p);
        int led_h = (int)center_h;
        if (led_h < 0) led_h = 0;
        if (led_h > 40) led_h = 40;

        int led_v = 220 + (int)(35.0f * sinf(p * 1.5f));
        if (led_v > 255) led_v = 255;
        if (led_v < 140) led_v = 140;

        int led_s = 250;
        effects_hsv_to_rgb(led_h, led_s, led_v, led_r, led_g, led_b);
    }
}
