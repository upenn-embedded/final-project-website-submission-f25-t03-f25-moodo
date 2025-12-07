#ifndef SHT40_H
#define SHT40_H

#include <stdint.h>

/**
 * Initialize TWI0 (I2C) for SHT40 on ATmega328PB:
 *   SDA0 -> PC4
 *   SCL0 -> PC5
 */
void SHT40_init(void);

/**
 * Trigger one measurement and read raw 16-bit temperature & humidity.
 * Returns 0 on success, non-zero on error.
 */
uint8_t SHT40_read_raw(uint16_t *raw_t, uint16_t *raw_rh);

/**
 * Convert raw values to engineering units according to SHT4x datasheet:
 *   T(°C) = -45 + 175 * raw_T  / 65535
 *   RH(%) =  -6 + 125 * raw_RH / 65535
 */
void SHT40_convert(uint16_t raw_t, uint16_t raw_rh,
                   float *temp_c, float *rh_percent);

/**
 * Convenience API:
 *   - triggers measurement
 *   - reads raw values
 *   - converts to °C and %RH
 * Returns 0 on success, non-zero on error.
 */
uint8_t SHT40_read(float *temp_c, float *rh_percent);

#endif  // SHT40_H
