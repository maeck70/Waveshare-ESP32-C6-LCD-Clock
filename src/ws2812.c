/**
 * @file ws2812.c
 * @brief Implementation of RMT driver for onboard WS2812 addressable RGB LED.
 */

#include "ws2812.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "WS2812";

/** RMT counter clock frequency: 10 MHz = 0.1 us per tick */
#define RMT_LED_RESOLUTION_HZ 10000000

/** WS2812 bit 0 symbol timing: 0.3 us high, 0.9 us low */
static const rmt_symbol_word_t ws2812_zero = {
    .level0 = 1,
    .duration0 = 3, // 0.3 us T0H
    .level1 = 0,
    .duration1 = 9, // 0.9 us T0L
};

/** WS2812 bit 1 symbol timing: 0.9 us high, 0.3 us low */
static const rmt_symbol_word_t ws2812_one = {
    .level0 = 1,
    .duration0 = 9, // 0.9 us T1H
    .level1 = 0,
    .duration1 = 3, // 0.3 us T1L
};

/** WS2812 reset symbol timing: >= 50 us low */
static const rmt_symbol_word_t ws2812_reset = {
    .level0 = 0,
    .duration0 = 250, // 25 us reset
    .level1 = 0,
    .duration1 = 250, // 25 us reset
};

/**
 * @brief RMT encoder callback converting raw RGB bytes into RMT symbol waveforms.
 *
 * @param data Pointer to raw byte array to transmit.
 * @param data_size Size of the raw data in bytes.
 * @param symbols_written Number of symbols already written.
 * @param symbols_free Free symbol slots in the destination buffer.
 * @param symbols Output symbol buffer.
 * @param done Output flag set to true when transmission is finished.
 * @param arg User callback argument.
 * @return Number of symbols encoded and written to the output buffer.
 */
static size_t encoder_callback(const void *data, size_t data_size,
                               size_t symbols_written, size_t symbols_free,
                               rmt_symbol_word_t *symbols, bool *done, void *arg)
{
    (void)arg;
    if (symbols_free < 8) {
        return 0;
    }
    size_t data_pos = symbols_written / 8;
    const uint8_t *data_bytes = (const uint8_t *)data;
    if (data_pos < data_size) {
        size_t symbol_pos = 0;
        for (int bitmask = 0x80; bitmask != 0; bitmask >>= 1) {
            if (data_bytes[data_pos] & bitmask) {
                symbols[symbol_pos++] = ws2812_one;
            } else {
                symbols[symbol_pos++] = ws2812_zero;
            }
        }
        return symbol_pos;
    } else {
        symbols[0] = ws2812_reset;
        *done = true;
        return 1;
    }
}

/** RMT transmit channel handle */
static rmt_channel_handle_t s_led_chan = NULL;

/** RMT symbol encoder handle */
static rmt_encoder_handle_t s_led_encoder = NULL;

esp_err_t ws2812_init(void) {
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = WS2812_GPIO_PIN,
        .mem_block_symbols = 64,
        .resolution_hz = RMT_LED_RESOLUTION_HZ,
        .trans_queue_depth = 4,
    };
    esp_err_t err = rmt_new_tx_channel(&tx_chan_config, &s_led_chan);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel: %s", esp_err_to_name(err));
        return err;
    }

    const rmt_simple_encoder_config_t encoder_cfg = {
        .callback = encoder_callback,
    };
    err = rmt_new_simple_encoder(&encoder_cfg, &s_led_encoder);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT encoder: %s", esp_err_to_name(err));
        return err;
    }

    err = rmt_enable(s_led_chan);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable RMT channel: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "WS2812 RMT driver initialized on GPIO %d", WS2812_GPIO_PIN);
    return ESP_OK;
}

esp_err_t ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    if (!s_led_chan || !s_led_encoder) return ESP_ERR_INVALID_STATE;
    // LED color sequence is RGB on this hardware
    uint8_t rgb[3] = {r, g, b};
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };
    esp_err_t err = rmt_transmit(s_led_chan, s_led_encoder, rgb, sizeof(rgb), &tx_config);
    if (err == ESP_OK) {
        rmt_tx_wait_all_done(s_led_chan, portMAX_DELAY);
    }
    return err;
}
