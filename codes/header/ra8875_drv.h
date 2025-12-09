#ifndef RA8875_DRV_H
#define RA8875_DRV_H

#include <stdint.h>

#define RA8875_DATAWRITE 0x00
#define RA8875_CMDWRITE  0x80

#define RA8875_PWRR   0x01
#define RA8875_MRWC   0x02
#define RA8875_PCSR   0x04
#define RA8875_SYSR   0x10

#define RA8875_HDWR   0x14
#define RA8875_HNDFTR 0x15
#define RA8875_HNDR   0x16
#define RA8875_HSTR   0x17
#define RA8875_HPWR   0x18
#define RA8875_VDHR0  0x19
#define RA8875_VDHR1  0x1A
#define RA8875_VNDR0  0x1B
#define RA8875_VNDR1  0x1C
#define RA8875_VSTR0  0x1D
#define RA8875_VSTR1  0x1E
#define RA8875_VPWR   0x1F

#define RA8875_HSAW0  0x30
#define RA8875_HSAW1  0x31
#define RA8875_VSAW0  0x32
#define RA8875_VSAW1  0x33
#define RA8875_HEAW0  0x34
#define RA8875_HEAW1  0x35
#define RA8875_VEAW0  0x36
#define RA8875_VEAW1  0x37

#define RA8875_CURH0  0x46
#define RA8875_CURH1  0x47
#define RA8875_CURV0  0x48
#define RA8875_CURV1  0x49

#define RA8875_PLLC1  0x88
#define RA8875_PLLC2  0x89

#define RA8875_MWCR0  0x40
#define RA8875_GPIOX  0xC7
#define RA8875_P1CR   0x8A
#define RA8875_P1DCR  0x8B

#define RA8875_PWRR_DISPON     0x80
#define RA8875_PWRR_DISPOFF    0x00
#define RA8875_PWRR_SLEEP      0x02
#define RA8875_PWRR_NORMAL     0x00
#define RA8875_PWRR_SOFTRESET  0x01

#define RA8875_SYSR_8BPP       0x00
#define RA8875_SYSR_16BPP      0x0C
#define RA8875_SYSR_MCU8       0x00
#define RA8875_SYSR_MCU16      0x03

#define RA8875_PCSR_PDATR      0x00
#define RA8875_PCSR_PDATL      0x80
#define RA8875_PCSR_CLK        0x00
#define RA8875_PCSR_2CLK       0x01
#define RA8875_PCSR_4CLK       0x02
#define RA8875_PCSR_8CLK       0x03

#define RA8875_MWCR0_GFXMODE   0x00
#define RA8875_MWCR0_LRTD      0x00

#define RA8875_P1CR_ENABLE     0x80
#define RA8875_P1CR_PWMOUT     0x00

void  spi_init(void);
void  ra8875_init_800x480(void);
void  ra8875_writeCommand(uint8_t cmd);
void  ra8875_writeData(uint8_t data);
void  ra8875_writeReg(uint8_t reg, uint8_t val);
void  ra8875_setXY(uint16_t x, uint16_t y);
void  ra8875_fillScreen(uint16_t color);
uint8_t ra8875_readReg(uint8_t reg);


#endif 
