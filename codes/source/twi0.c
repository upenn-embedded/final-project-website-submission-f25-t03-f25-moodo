/*
 * File:   twi0.c
 * Author: houjie
 *
 * Created on November 18, 2025, 6:24 PM
 */


#define F_CPU 16000000UL

#include <avr/io.h>
#include "twi0.h"

void TWI0_init(void)
{
    // Enable TWI0 clock
    PRR0 &= ~(1 << PRTWI0);

    // Prescaler = 1
    TWSR0 = 0x00;

    // SCL ≈ 100 kHz:
    // F_SCL = F_CPU / (16 + 2 * TWBR0)
    // For F_CPU = 16MHz, TWBR0 ≈ 72
    TWBR0 = 72;

    // Enable TWI0
    TWCR0 = (1 << TWEN);
}

// Send START + SLA+R/W
void TWI0_start(uint8_t addr_rw)
{
    // START
    TWCR0 = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT))) {;}

    // SLA+R/W
    TWDR0 = addr_rw;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT))) {;}
}

// STOP condition
void TWI0_stop(void)
{
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR0 & (1 << TWSTO)) {;}
}

// Write one byte
void TWI0_write(uint8_t data)
{
    TWDR0 = data;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT))) {;}
}

// Read one byte + ACK (more data to come)
uint8_t TWI0_read_ack(void)
{
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR0 & (1 << TWINT))) {;}
    return TWDR0;
}

// Read one byte + NACK (last byte)
uint8_t TWI0_read_nack(void)
{
    TWCR0 = (1 << TWINT) | (1 << TWEN);   // no TWEA -> NACK
    while (!(TWCR0 & (1 << TWINT))) {;}
    return TWDR0;
}
