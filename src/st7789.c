/**
 * @file st7789.c
 * @brief SPI driver and framebuffer graphics operations for ST7789 display controller.
 */

#include "st7789.h"

#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ST7789";

static spi_device_handle_t s_spi_dev;
static uint16_t s_framebuffer[LCD_WIDTH * LCD_HEIGHT];
static uint8_t s_current_rotation = 0;

// Command and Data transmission helpers
static void lcd_cmd(uint8_t cmd) {
    gpio_set_level(LCD_DC_PIN, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_polling_transmit(s_spi_dev, &t);
}

static void lcd_data(const uint8_t *data, size_t len) {
    if (len == 0) return;
    gpio_set_level(LCD_DC_PIN, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_polling_transmit(s_spi_dev, &t);
}

static void lcd_data_byte(uint8_t d) {
    lcd_data(&d, 1);
}

static void lcd_reset(void) {
    gpio_set_level(LCD_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(30));
    gpio_set_level(LCD_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(150));
}

// Convert HSV to byte-swapped RGB565 for ST7789
uint16_t hsv2rgb565(int h, int s, int v) {
    if (s == 0) {
        return color565(v, v, v);
    }
    h = h % 360;
    if (h < 0) h += 360;

    int region = h / 60;
    int remainder = (h - (region * 60)) * 6;

    int p = (v * (255 - s)) >> 8;
    int q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    int t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    int r = 0, g = 0, b = 0;
    switch (region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return color565((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

void st7789_set_rotation(uint8_t rotation) {
    s_current_rotation = rotation & 1;
    lcd_cmd(0x36); // MADCTL
    // 0x70: MX | MV | ML (RGB color order)
    // 0xA0: MY | MV      (RGB color order)
    if (s_current_rotation == 0) {
        lcd_data_byte(0x70);
    } else {
        lcd_data_byte(0xA0);
    }
}

void st7789_set_backlight(bool enable) {
    gpio_set_level(LCD_BL_PIN, enable ? 1 : 0);
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    const uint8_t caset[4] = {
        (uint8_t)((x0 + LCD_X_OFFSET) >> 8),
        (uint8_t)((x0 + LCD_X_OFFSET) & 0xFF),
        (uint8_t)((x1 + LCD_X_OFFSET) >> 8),
        (uint8_t)((x1 + LCD_X_OFFSET) & 0xFF),
    };
    lcd_cmd(0x2A); // CASET
    lcd_data(caset, 4);

    const uint8_t raset[4] = {
        (uint8_t)((y0 + LCD_Y_OFFSET) >> 8),
        (uint8_t)((y0 + LCD_Y_OFFSET) & 0xFF),
        (uint8_t)((y1 + LCD_Y_OFFSET) >> 8),
        (uint8_t)((y1 + LCD_Y_OFFSET) & 0xFF),
    };
    lcd_cmd(0x2B); // RASET
    lcd_data(raset, 4);

    lcd_cmd(0x2C); // RAMWR
}

void st7789_init(uint8_t rotation) {
    ESP_LOGI(TAG, "Initializing ST7789 display controller...");

    // Configure DC, RST, and BL GPIO pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_DC_PIN) | (1ULL << LCD_RST_PIN) | (1ULL << LCD_BL_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(LCD_BL_PIN, 0); // Keep backlight off during init

    // Initialize SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = LCD_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320 * 20 * 2 + 8, // DMA transfer chunk size
    };
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // Add ST7789 SPI device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000, // 40 MHz
        .mode = 0,                         // SPI Mode 0
        .spics_io_num = LCD_CS_PIN,
        .queue_size = 8,
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &s_spi_dev);
    ESP_ERROR_CHECK(ret);

    // Hardware reset
    lcd_reset();

    // ST7789 Initialization Sequence
    lcd_cmd(0x01); // Software Reset
    vTaskDelay(pdMS_TO_TICKS(150));

    lcd_cmd(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));

    lcd_cmd(0x3A); // Interface Pixel Format (COLMOD)
    lcd_data_byte(0x55); // 16-bit / pixel (RGB565)
    vTaskDelay(pdMS_TO_TICKS(10));

    st7789_set_rotation(rotation);

    lcd_cmd(0x21); // Display Inversion ON (required for correct IPS colors)
    vTaskDelay(pdMS_TO_TICKS(10));

    lcd_cmd(0xB2); // Porch Setting
    uint8_t porctrl[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    lcd_data(porctrl, sizeof(porctrl));

    lcd_cmd(0xB7); // Gate Control
    lcd_data_byte(0x35);

    lcd_cmd(0xBB); // VCOM Setting
    lcd_data_byte(0x19);

    lcd_cmd(0xC0); // LCM Control
    lcd_data_byte(0x2C);

    lcd_cmd(0xC2); // VDV and VRH Command Enable
    lcd_data_byte(0x01);

    lcd_cmd(0xC3); // VRH Set
    lcd_data_byte(0x12);

    lcd_cmd(0xC4); // VDV Set
    lcd_data_byte(0x20);

    lcd_cmd(0xC6); // Frame Rate Control in Normal Mode
    lcd_data_byte(0x0F); // 60 Hz

    lcd_cmd(0xD0); // Power Control 1
    uint8_t pwctrl[] = {0xA4, 0xA1};
    lcd_data(pwctrl, sizeof(pwctrl));

    lcd_cmd(0xE0); // Positive Voltage Gamma Control
    uint8_t pgam[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
    lcd_data(pgam, sizeof(pgam));

    lcd_cmd(0xE1); // Negative Voltage Gamma Control
    uint8_t ngam[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
    lcd_data(ngam, sizeof(ngam));

    // Clear screen to black before turning on display
    fb_clear(0x0000);
    st7789_flush();

    lcd_cmd(0x29); // Display ON
    vTaskDelay(pdMS_TO_TICKS(100));

    // Turn on backlight
    st7789_set_backlight(true);
    ESP_LOGI(TAG, "ST7789 initialization complete.");
}

void fb_clear(uint16_t color) {
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        s_framebuffer[i] = color;
    }
}

void fb_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
    s_framebuffer[y * LCD_WIDTH + x] = color;
}

void fb_fill_rect(int x, int y, int w, int h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT || x + w <= 0 || y + h <= 0) return;
    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > LCD_WIDTH) ? LCD_WIDTH : x + w;
    int y1 = (y + h > LCD_HEIGHT) ? LCD_HEIGHT : y + h;

    for (int j = y0; j < y1; j++) {
        uint16_t *line = &s_framebuffer[j * LCD_WIDTH];
        for (int i = x0; i < x1; i++) {
            line[i] = color;
        }
    }
}

void fb_draw_line_h(int x0, int x1, int y, uint16_t color) {
    if (y < 0 || y >= LCD_HEIGHT) return;
    if (x0 > x1) { int tmp = x0; x0 = x1; x1 = tmp; }
    if (x0 < 0) x0 = 0;
    if (x1 >= LCD_WIDTH) x1 = LCD_WIDTH - 1;
    uint16_t *line = &s_framebuffer[y * LCD_WIDTH];
    for (int i = x0; i <= x1; i++) {
        line[i] = color;
    }
}

void st7789_flush(void) {
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    gpio_set_level(LCD_DC_PIN, 1);

    const size_t total_bytes = LCD_WIDTH * LCD_HEIGHT * 2;
    const size_t chunk_size = 320 * 20 * 2; // 12,800 bytes per transfer
    size_t sent = 0;

    while (sent < total_bytes) {
        size_t len = total_bytes - sent;
        if (len > chunk_size) len = chunk_size;

        spi_transaction_t t = {
            .length = len * 8,
            .tx_buffer = ((const uint8_t *)s_framebuffer) + sent,
        };
        spi_device_polling_transmit(s_spi_dev, &t);
        sent += len;
    }
}
