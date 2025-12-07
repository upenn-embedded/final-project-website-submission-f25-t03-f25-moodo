/* 
 * File:   ds1307.h
 * Author: houjie
 *
 * Created on November 18, 2025, 6:16 PM
 */
#ifndef DS1307_H
#define DS1307_H

#include <stdint.h>

typedef struct {
    uint8_t second;  // 0–59
    uint8_t minute;  // 0–59
    uint8_t hour;    // 0–23
    uint8_t day;     // 1–7 (day of week)
    uint8_t date;    // 1–31
    uint8_t month;   // 1–12
    uint8_t year;    // 0–99 (e.g. 25 = 2025)
} ds1307_time_t;

void   DS1307_init(void);
uint8_t DS1307_read_time(ds1307_time_t *t);
uint8_t DS1307_set_time(const ds1307_time_t *t);

#endif // DS1307_H
