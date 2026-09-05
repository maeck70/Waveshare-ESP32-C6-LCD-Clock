/**
 * @file ui.c
 * @brief Implementation of UI rendering and font rasterization.
 */

#include "ui.h"

#include <stddef.h>
#include <stdint.h>
#include "app_font_data.h"
#include "esp_log.h"
#include "st7789.h"

static const char *TAG = "UI";

/** Current display orientation (0: landscape, 1: flipped landscape) */
static uint8_t s_rotation = 0;

void ui_init(uint8_t initial_rotation) {
    s_rotation = initial_rotation;
    st7789_init(s_rotation);
}

void ui_set_rotation(uint8_t rotation) {
    s_rotation = rotation;
    st7789_set_rotation(s_rotation);
    ESP_LOGI(TAG, "Orientation set to: %d", s_rotation);
}

uint8_t ui_get_rotation(void) {
    return s_rotation;
}

void ui_toggle_rotation(void) {
    ui_set_rotation(1 - s_rotation);
}

/**
 * @brief Render a single time digit/colon character using alpha-blending onto black.
 *
 * @param x Top-left X coordinate.
 * @param y Top-left Y coordinate.
 * @param c Character to render.
 * @param r_fg Foreground red channel (0..255).
 * @param g_fg Foreground green channel (0..255).
 * @param b_fg Foreground blue channel (0..255).
 */
static void draw_time_char(int x, int y, char c, uint8_t r_fg, uint8_t g_fg, uint8_t b_fg) {
    const glyph_desc_t *g = get_time_glyph(c);
    if (!g) return;

    for (int row = 0; row < g->height; row++) {
        int py = y + row;
        if (py < 0 || py >= LCD_HEIGHT) continue;
        const uint8_t *src_line = &g->bitmap[row * g->width];
        for (int col = 0; col < g->width; col++) {
            int px = x + col;
            if (px < 0 || px >= LCD_WIDTH) continue;
            uint8_t a = src_line[col];
            if (a == 0) continue;
            if (a >= 250) {
                fb_draw_pixel(px, py, color565(r_fg, g_fg, b_fg));
            } else {
                uint8_t r = (uint8_t)(((uint16_t)r_fg * a) >> 8);
                uint8_t g_val = (uint8_t)(((uint16_t)g_fg * a) >> 8);
                uint8_t b = (uint8_t)(((uint16_t)b_fg * a) >> 8);
                fb_draw_pixel(px, py, color565(r, g_val, b));
            }
        }
    }
}

/**
 * @brief Calculate total horizontal pixel width of a time string.
 *
 * @param str Null-terminated time string.
 * @param spacing Pixel gap between adjacent characters.
 * @return Total width in pixels.
 */
static int get_time_width(const char *str, int spacing) {
    int w = 0, count = 0;
    while (*str) {
        const glyph_desc_t *g = get_time_glyph(*str);
        if (g) {
            w += g->width;
            count++;
        }
        str++;
    }
    if (count > 1) w += (count - 1) * spacing;
    return w;
}

/**
 * @brief Render a single date character using alpha-blending onto black.
 *
 * @param x Top-left X coordinate.
 * @param y Top-left Y coordinate.
 * @param c Character to render.
 * @param r_fg Foreground red channel (0..255).
 * @param g_fg Foreground green channel (0..255).
 * @param b_fg Foreground blue channel (0..255).
 */
static void draw_date_char(int x, int y, char c, uint8_t r_fg, uint8_t g_fg, uint8_t b_fg) {
    const glyph_desc_t *g = get_date_glyph(c);
    if (!g) return;

    for (int row = 0; row < g->height; row++) {
        int py = y + row;
        if (py < 0 || py >= LCD_HEIGHT) continue;
        const uint8_t *src_line = &g->bitmap[row * g->width];
        for (int col = 0; col < g->width; col++) {
            int px = x + col;
            if (px < 0 || px >= LCD_WIDTH) continue;
            uint8_t a = src_line[col];
            if (a == 0) continue;
            if (a >= 250) {
                fb_draw_pixel(px, py, color565(r_fg, g_fg, b_fg));
            } else {
                uint8_t r = (uint8_t)(((uint16_t)r_fg * a) >> 8);
                uint8_t g_val = (uint8_t)(((uint16_t)g_fg * a) >> 8);
                uint8_t b = (uint8_t)(((uint16_t)b_fg * a) >> 8);
                fb_draw_pixel(px, py, color565(r, g_val, b));
            }
        }
    }
}

/**
 * @brief Calculate total horizontal pixel width of a date string.
 *
 * @param str Null-terminated date string.
 * @param spacing Pixel gap between adjacent non-space characters.
 * @return Total width in pixels.
 */
static int get_date_width(const char *str, int spacing) {
    int w = 0;
    while (*str) {
        if (*str == ' ') {
            w += 7; // Fixed space character width
        } else {
            const glyph_desc_t *g = get_date_glyph(*str);
            if (g) {
                w += g->width + spacing;
            }
        }
        str++;
    }
    return w;
}

/**
 * @brief Draw an entire string using the date font.
 *
 * @param x Starting X coordinate.
 * @param y Starting Y coordinate.
 * @param str Null-terminated string to render.
 * @param r Red component (0..255).
 * @param g Green component (0..255).
 * @param b Blue component (0..255).
 * @param spacing Pixel gap between adjacent glyphs.
 */
static void draw_date_string(int x, int y, const char *str, uint8_t r, uint8_t g, uint8_t b, int spacing) {
    int cur_x = x;
    while (*str) {
        if (*str == ' ') {
            cur_x += 7;
        } else {
            const glyph_desc_t *glyph = get_date_glyph(*str);
            if (glyph) {
                draw_date_char(cur_x, y, *str, r, g, b);
                cur_x += glyph->width + spacing;
            }
        }
        str++;
    }
}

void ui_render_scene(const char *time_str,
                     const uint8_t time_r[8],
                     const uint8_t time_g[8],
                     const uint8_t time_b[8],
                     const char *date_str) {
    // 1. Pure black background
    fb_clear(0x0000);

    // 2. Render Top Row (Time: "HH:MM:SS")
    const int time_spacing = 3;
    const int total_time_w = get_time_width(time_str, time_spacing);
    const int time_x0 = (LCD_WIDTH - total_time_w) / 2; // Centered
    const int time_y = 22;                             // Top margin: 22 px (Y: 22..90)

    int cur_x = time_x0;
    for (int i = 0; i < 8 && time_str[i] != '\0'; i++) {
        char c = time_str[i];
        draw_time_char(cur_x, time_y, c, time_r[i], time_g[i], time_b[i]);
        const glyph_desc_t *g = get_time_glyph(c);
        if (g) {
            cur_x += g->width + time_spacing;
        }
    }

    // 3. Render Bottom Row (Date: e.g. "September 3, 2026")
    // Exact 50% grey (128, 128, 128), height = 28 px
    if (date_str) {
        const int date_spacing = 2;
        const int total_date_w = get_date_width(date_str, date_spacing);
        const int date_x0 = (LCD_WIDTH - total_date_w) / 2; // Centered
        const int date_y = 118;                             // Y: 118..146

        draw_date_string(date_x0, date_y, date_str, 128, 128, 128, date_spacing);
    }

    // 4. Transfer to display over SPI
    st7789_flush();
}
