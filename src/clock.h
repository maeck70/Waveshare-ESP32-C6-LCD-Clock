/**
 * @file clock.h
 * @brief Timekeeping and serial synchronization module.
 *
 * Manages hours, minutes, seconds, and date string state, with second-based
 * elapsed time tracking and UART serial synchronization.
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize clock state using compilation timestamp (__TIME__) and default date.
 */
void clock_init(void);

/**
 * @brief Advance clock counters based on elapsed time from high-resolution timer.
 */
void clock_tick(void);

/**
 * @brief Retrieve current hours, minutes, and seconds.
 *
 * @param[out] hours Pointer to store hours (0..23), or NULL if not needed.
 * @param[out] minutes Pointer to store minutes (0..59), or NULL if not needed.
 * @param[out] seconds Pointer to store seconds (0..59), or NULL if not needed.
 */
void clock_get_time(int *hours, int *minutes, int *seconds);

/**
 * @brief Format current time as null-terminated "HH:MM:SS".
 *
 * @param[out] buf Output buffer.
 * @param[in]  max_len Capacity of the output buffer in bytes (must be >= 9).
 */
void clock_get_time_str(char *buf, size_t max_len);

/**
 * @brief Retrieve pointer to current date string.
 *
 * @return Null-terminated date string.
 */
const char *clock_get_date_str(void);

/**
 * @brief Manually set the current time.
 *
 * @param hours Hours value (0..23).
 * @param minutes Minutes value (0..59).
 * @param seconds Seconds value (0..59).
 */
void clock_set_time(int hours, int minutes, int seconds);

/**
 * @brief Manually set the date string.
 *
 * @param date_str Null-terminated date string.
 */
void clock_set_date(const char *date_str);

/**
 * @brief Start background FreeRTOS task listening for time/date updates over USB serial.
 *
 * Accepts lines formatted as:
 * - "HH:MM:SS\n" to set the time.
 * - "DATE=<date_str>\n" to set the date string.
 */
void clock_serial_sync_init(void);

#ifdef __cplusplus
}
#endif
