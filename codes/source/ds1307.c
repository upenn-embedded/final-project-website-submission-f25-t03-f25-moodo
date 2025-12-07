/*
 * File:   ds1307.c
 * Author: houjie
 *
 * Created on November 18, 2025, 6:19 PM
 */
#include <avr/io.h>
#include "ds1307.h"
#include "twi0.h"

#define DS1307_ADDR  0x68  // 7-bit I2C address

// ----- BCD helpers -----

static uint8_t bcd_to_dec(uint8_t bcd)
{
    return (uint8_t)(((bcd >> 4) * 10) + (bcd & 0x0F));
}

static uint8_t dec_to_bcd(uint8_t dec)
{
    return (uint8_t)(((dec / 10) << 4) | (dec % 10));
}

// ----- Public API -----

void DS1307_init(void)
{
    // Assumes TWI0_init() was called already (via SHT40_init or explicitly).
    // Clear CH (clock halt) bit in seconds register.

    // Read seconds
    TWI0_start((DS1307_ADDR << 1) | 0);  // write
    TWI0_write(0x00);                    // seconds register
    TWI0_start((DS1307_ADDR << 1) | 1);  // read
    uint8_t sec = TWI0_read_nack();
    TWI0_stop();

    sec &= 0x7F; // clear CH bit

    // Write back seconds
    TWI0_start((DS1307_ADDR << 1) | 0);  // write
    TWI0_write(0x00);                    // seconds register
    TWI0_write(sec);
    TWI0_stop();
}

uint8_t DS1307_read_time(ds1307_time_t *t)
{
    if (!t) return 1;

    // Point to register 0x00 (seconds)
    TWI0_start((DS1307_ADDR << 1) | 0);  // write
    TWI0_write(0x00);                    // start at 0x00

    // Repeated start for read
    TWI0_start((DS1307_ADDR << 1) | 1);  // read

    uint8_t raw_sec   = TWI0_read_ack();
    uint8_t raw_min   = TWI0_read_ack();
    uint8_t raw_hour  = TWI0_read_ack();
    uint8_t raw_day   = TWI0_read_ack();
    uint8_t raw_date  = TWI0_read_ack();
    uint8_t raw_month = TWI0_read_ack();
    uint8_t raw_year  = TWI0_read_nack();
    TWI0_stop();

    raw_sec &= 0x7F; // clear CH bit just in case

    t->second = bcd_to_dec(raw_sec);
    t->minute = bcd_to_dec(raw_min);

    // 24h vs 12h mode
    if (raw_hour & 0x40) {
        // 12-hour mode
        uint8_t am_pm = (raw_hour & 0x20) ? 1 : 0; // 1 = PM
        uint8_t h12   = bcd_to_dec(raw_hour & 0x1F);
        if (am_pm && h12 != 12) h12 += 12;
        if (!am_pm && h12 == 12) h12 = 0;
        t->hour = h12;
    } else {
        // 24-hour mode
        t->hour = bcd_to_dec(raw_hour & 0x3F);
    }

    t->day   = bcd_to_dec(raw_day   & 0x07);
    t->date  = bcd_to_dec(raw_date  & 0x3F);
    t->month = bcd_to_dec(raw_month & 0x1F);
    t->year  = bcd_to_dec(raw_year);

    return 0;
}

uint8_t DS1307_set_time(const ds1307_time_t *t)
{
    if (!t) return 1;

    uint8_t sec   = dec_to_bcd(t->second) & 0x7F;  // CH=0
    uint8_t min   = dec_to_bcd(t->minute);
    uint8_t hour  = dec_to_bcd(t->hour) & 0x3F;    // 24h mode
    uint8_t day   = dec_to_bcd(t->day   & 0x07);
    uint8_t date  = dec_to_bcd(t->date  & 0x3F);
    uint8_t month = dec_to_bcd(t->month & 0x1F);
    uint8_t year  = dec_to_bcd(t->year);

    // Write starting at register 0x00
    TWI0_start((DS1307_ADDR << 1) | 0); // write
    TWI0_write(0x00);                   // start address

    TWI0_write(sec);
    TWI0_write(min);
    TWI0_write(hour);
    TWI0_write(day);
    TWI0_write(date);
    TWI0_write(month);
    TWI0_write(year);

    TWI0_stop();
    return 0;
}
