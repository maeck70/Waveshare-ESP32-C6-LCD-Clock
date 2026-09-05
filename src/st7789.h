/**
 * @file st7789.h
 * @brief Hardware definitions, graphics primitives, and SPI driver for the ST7789 LCD.
 *
 * Configured for the Waveshare ESP32-C6-LCD-1.47 IPS display panel (320x172 landscape).
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Display width in landscape orientation */
#define LCD_WIDTH       320

/** Display height in landscape orientation */
#define LCD_HEIGHT      172

/** Waveshare ESP32-C6-LCD-1.47 Hardware Pin Configuration */
#define LCD_MOSI_PIN    6
#define LCD_SCLK_PIN    7
#define LCD_CS_PIN      14
#define LCD_DC_PIN      15
#define LCD_RST_PIN     21
#define LCD_BL_PIN      22
#define BOOT_BTN_PIN    9

/** ST7789 Landscape Window Controller Offsets */
#define LCD_X_OFFSET    0
#define LCD_Y_OFFSET    34

/**
 * @brief Convert standard RGB888 components to byte-swapped RGB565 for big-endian SPI transmission.
 *
 * @param r Red component (0..255).
 * @param g Green component (0..255).
 * @param b Blue component (0..255).
 * @return Byte-swapped 16-bit RGB565 color value.
 */
static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t c = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    return (uint16_t)((c >> 8) | (c << 8));
}

/**
 * @brief Convert HSV color values to byte-swapped RGB565.
 *
 * @param h Hue (0..360).
 * @param s Saturation (0..255).
 * @param v Value (0..255).
 * @return Byte-swapped 16-bit RGB565 color value.
 */
uint16_t hsv2rgb565(int h, int s, int v);

/**
 * @brief Initialize ST7789 display controller, SPI bus, and backlight.
 *
 * @param rotation 0: standard landscape, 1: 180-deg flipped landscape.
 */
void st7789_init(uint8_t rotation);

/**
 * @brief Set display rotation / orientation.
 *
 * @param rotation 0: standard landscape, 1: 180-deg flipped landscape.
 */
void st7789_set_rotation(uint8_t rotation);

/**
 * @brief Enable or disable LCD backlight.
 *
 * @param enable True to turn on, false to turn off.
 */
void st7789_set_backlight(bool enable);

/**
 * @brief Clear framebuffer with a solid color.
 *
 * @param color 16-bit byte-swapped RGB565 color.
 */
void fb_clear(uint16_t color);

/**
 * @brief Draw a single pixel in the framebuffer.
 *
 * @param x Horizontal pixel index (0..LCD_WIDTH-1).
 * @param y Vertical pixel index (0..LCD_HEIGHT-1).
 * @param color 16-bit byte-swapped RGB565 color.
 */
void fb_draw_pixel(int x, int y, uint16_t color);

/**
 * @brief Draw a filled rectangle in the framebuffer.
 *
 * @param x Top-left X coordinate.
 * @param y Top-left Y coordinate.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @param color 16-bit byte-swapped RGB565 color.
 */
void fb_fill_rect(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Draw a horizontal line in the framebuffer.
 *
 * @param x0 Starting X coordinate.
 * @param x1 Ending X coordinate.
 * @param y Horizontal row index.
 * @param color 16-bit byte-swapped RGB565 color.
 */
void fb_draw_line_h(int x0, int x1, int y, uint16_t color);

/**
 * @brief Draw an ASCII character using the built-in 8x16 bitmap font.
 *
 * @param x Starting X coordinate.
 * @param y Starting Y coordinate.
 * @param c ASCII character to draw.
 * @param color 16-bit byte-swapped RGB565 color.
 * @param sx Horizontal scale multiplier (e.g. 1, 2).
 * @param sy Vertical scale multiplier (e.g. 1, 2).
 */
void fb_draw_char(int x, int y, char c, uint16_t color, int sx, int sy);

/**
 * @brief Draw a string using the built-in 8x16 bitmap font.
 *
 * @param x Starting X coordinate.
 * @param y Starting Y coordinate.
 * @param str Null-terminated string to draw.
 * @param color 16-bit byte-swapped RGB565 color.
 * @param sx Horizontal scale multiplier.
 * @param sy Vertical scale multiplier.
 * @param spacing Additional pixel spacing between characters.
 */
void fb_draw_string(int x, int y, const char *str, uint16_t color, int sx, int sy, int spacing);

/**
 * @brief Calculate pixel width of a string rendered with the built-in 8x16 font.
 *
 * @param str Null-terminated string.
 * @param sx Horizontal scale multiplier.
 * @param spacing Additional pixel spacing between characters.
 * @return Total width in pixels.
 */
int fb_get_string_width(const char *str, int sx, int spacing);

/**
 * @brief Flush internal RAM framebuffer to the ST7789 LCD over SPI DMA / polling.
 */
void st7789_flush(void);

#ifdef __cplusplus
}
#endif
