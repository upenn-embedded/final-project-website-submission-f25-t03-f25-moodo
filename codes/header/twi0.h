/* 
 * File:   twi0.h
 * Author: houjie
 *
 * Created on November 18, 2025, 6:24 PM
 */
#ifndef TWI0_H
#define TWI0_H

#include <stdint.h>

// Initialize TWI0 on PC4 (SDA0) / PC5 (SCL0)
void TWI0_init(void);

// Send START + SLA+R/W
void TWI0_start(uint8_t addr_rw);

// STOP condition
void TWI0_stop(void);

// Write one byte
void TWI0_write(uint8_t data);

// Read one byte + ACK (more data to come)
uint8_t TWI0_read_ack(void);

// Read one byte + NACK (last byte)
uint8_t TWI0_read_nack(void);

#endif // TWI0_H
