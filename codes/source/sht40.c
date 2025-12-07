#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "sht40.h"
#include "twi0.h"


/* ========= SHT40 macros ========= */

#define SHT4X_I2C_ADDR   0x44  // 7-bit I2C address
// High-precision, no-heat measurement command (0xFD from datasheet)
#define SHT4X_CMD_MEASURE_HIGH_PREC_NOHEAT  0xFD





/* ========= Public SHT40 functions ========= */
void SHT40_init(void)
{
    TWI0_init();
}

uint8_t SHT40_read_raw(uint16_t *raw_t, uint16_t *raw_rh)
{
    uint8_t t_msb, t_lsb, t_crc;
    uint8_t rh_msb, rh_lsb, rh_crc;

    // 1) Send measurement command 0xFD (high precision, no heat)
    TWI0_start((SHT4X_I2C_ADDR << 1) | 0);   // write
    TWI0_write(SHT4X_CMD_MEASURE_HIGH_PREC_NOHEAT);
    TWI0_stop();

    // 2) Wait for conversion (typ. 9 ms, use 10 ms)
    _delay_ms(10);

    // 3) Read back 6 bytes: T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC
    TWI0_start((SHT4X_I2C_ADDR << 1) | 1);   // read

    t_msb  = TWI0_read_ack();
    t_lsb  = TWI0_read_ack();
    t_crc  = TWI0_read_ack();   // CRC ignored

    rh_msb = TWI0_read_ack();
    rh_lsb = TWI0_read_ack();
    rh_crc = TWI0_read_nack();

    TWI0_stop();

    (void)t_crc;
    (void)rh_crc;

    *raw_t  = ((uint16_t)t_msb  << 8) | t_lsb;
    *raw_rh = ((uint16_t)rh_msb << 8) | rh_lsb;

    return 0;
}


void SHT40_convert(uint16_t raw_t, uint16_t raw_rh,
                   float *temp_c, float *rh_percent)
{

    *temp_c = -45.0f + 175.0f * ((float)raw_t / 65535.0f);
    

    *rh_percent = -6.0f + 125.0f * ((float)raw_rh / 65535.0f);
    
}

uint8_t SHT40_read(float *temp_c, float *rh_percent)
{
    uint16_t raw_t, raw_rh;
    uint8_t status = SHT40_read_raw(&raw_t, &raw_rh);
    if (status != 0) {
        return status;
    }

    SHT40_convert(raw_t, raw_rh, temp_c, rh_percent);
    return 0;
}
