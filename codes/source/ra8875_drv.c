#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "ra8875_drv.h"

/* ATmega328PB SPI0: MOSI=PB3, MISO=PB4, SCK=PB5, CS=PB2 */
#define RA8875_CS_PORT  PORTB
#define RA8875_CS_DDR   DDRB
#define RA8875_CS_PIN   PB2

#define RA8875_RST_PORT PORTB
#define RA8875_RST_DDR  DDRB
#define RA8875_RST_PIN  PB1

/* ?? LITE ??? PD6????????? */
#define LITE_PORT  PORTD
#define LITE_DDR   DDRD
#define LITE_PIN   PD6

/* ---------- SPI ?? ---------- */
static void ra8875_select(void)
{
    RA8875_CS_PORT &= ~(1 << RA8875_CS_PIN);
}

static void ra8875_deselect(void)
{
    RA8875_CS_PORT |= (1 << RA8875_CS_PIN);
}

static uint8_t spi_transfer(uint8_t data)
{
    SPDR0 = data;
    while (!(SPSR0 & (1 << SPIF)))
        ;
    return SPDR0;
}

void spi_init(void)
{
    // MOSI, SCK, CS ???MISO ??
    DDRB |= (1 << PB3) | (1 << PB5) | (1 << RA8875_CS_PIN);
    DDRB &= ~(1 << PB4);

    // SPI0: ??????????0?SCK = F_CPU/4?? x2 = F_CPU/2
    SPCR0 = (1 << SPE) | (1 << MSTR);
    SPSR0 = (1 << SPI2X);

    // CS ????
    RA8875_CS_PORT |= (1 << RA8875_CS_PIN);
}

/* ---------- RA8875 ???? ---------- */

void ra8875_writeCommand(uint8_t cmd)
{
    ra8875_select();
    spi_transfer(RA8875_CMDWRITE); // ????
    spi_transfer(cmd);
    ra8875_deselect();
}

void ra8875_writeData(uint8_t data)
{
    ra8875_select();
    spi_transfer(RA8875_DATAWRITE); // ????
    spi_transfer(data);
    ra8875_deselect();
}

void ra8875_writeReg(uint8_t reg, uint8_t val)
{
    ra8875_writeCommand(reg);
    ra8875_writeData(val);
}

/* ---------- ?? & ??? ---------- */

static void ra8875_hard_reset(void)
{
    RA8875_RST_DDR |= (1 << RA8875_RST_PIN);

    RA8875_RST_PORT &= ~(1 << RA8875_RST_PIN);
    _delay_ms(1);
    RA8875_RST_PORT |= (1 << RA8875_RST_PIN);
    _delay_ms(10);
}

void ra8875_setXY(uint16_t x, uint16_t y)
{
    ra8875_writeReg(RA8875_CURH0, x & 0xFF);
    ra8875_writeReg(RA8875_CURH1, (x >> 8) & 0x03);
    ra8875_writeReg(RA8875_CURV0, y & 0xFF);
    ra8875_writeReg(RA8875_CURV1, (y >> 8) & 0x01);
}

void ra8875_init_800x480(void)
{
    ra8875_hard_reset();     // ????
    _delay_ms(10);

    /* 1. PLL ??? ?? ?????? */
    ra8875_writeReg(RA8875_PLLC1, 0x0A);
    _delay_ms(1);
    ra8875_writeReg(RA8875_PLLC2, 0x02);
    _delay_ms(1);

    /* 2. ???? */
    ra8875_writeReg(RA8875_PWRR, RA8875_PWRR_SOFTRESET);
    _delay_ms(5);
    ra8875_writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL);
    _delay_ms(5);

    /* 3. ?????16-bit color, 8-bit MCU */
    ra8875_writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);
    _delay_ms(1);

    /* 4. PCLK: sysclk/4 ?????? */
    ra8875_writeReg(RA8875_PCSR, 0x82);    // PDAT=low, PCLK=sysclk/4
    _delay_ms(1);

    /* 5. 800×480 timing */
    // Horizontal
    ra8875_writeReg(RA8875_HDWR,   0x63);
    ra8875_writeReg(RA8875_HNDFTR, 0x00);
    ra8875_writeReg(RA8875_HNDR,   0x03);
    ra8875_writeReg(RA8875_HSTR,   0x03);
    ra8875_writeReg(RA8875_HPWR,   0x0B);

    // Vertical
    ra8875_writeReg(RA8875_VDHR0,  0xDF);
    ra8875_writeReg(RA8875_VDHR1,  0x01);
    ra8875_writeReg(RA8875_VNDR0,  0x20);
    ra8875_writeReg(RA8875_VNDR1,  0x00);
    ra8875_writeReg(RA8875_VSTR0,  0x16);
    ra8875_writeReg(RA8875_VSTR1,  0x00);
    ra8875_writeReg(RA8875_VPWR,   0x01);

    /* 6. Active window = full screen */
    ra8875_writeReg(RA8875_HSAW0, 0x00);
    ra8875_writeReg(RA8875_HSAW1, 0x00);
    ra8875_writeReg(RA8875_HEAW0, 0x1F);
    ra8875_writeReg(RA8875_HEAW1, 0x03);

    ra8875_writeReg(RA8875_VSAW0, 0x00);
    ra8875_writeReg(RA8875_VSAW1, 0x00);
    ra8875_writeReg(RA8875_VEAW0, 0xDF);
    ra8875_writeReg(RA8875_VEAW1, 0x01);

    /* 7. GPIOX ?? TFT ?? */
    ra8875_writeReg(RA8875_GPIOX, 0x01);

    /* 8. PWM ???? */
    ra8875_writeReg(RA8875_P1CR, 0x8B);   // enable PWM1 @ PWM mode
    ra8875_writeReg(RA8875_P1DCR, 255);   // ??????

    /* 9. ??? */
    ra8875_writeReg(RA8875_PWRR, RA8875_PWRR_DISPON | RA8875_PWRR_NORMAL);

    _delay_ms(10);
}


void ra8875_fillScreen(uint16_t color)
{
    uint32_t i;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    // ????? (0,0)
    ra8875_setXY(0, 0);

    // ?????
    ra8875_writeCommand(RA8875_MRWC);

    // ?? Adafruit ?????????? writeData ??
    for (i = 0; i < 800UL * 480UL; i++) {
        ra8875_writeData(hi);    // ???? 0x00 + hi
        ra8875_writeData(lo);    // ???? 0x00 + lo
    }
}

uint8_t ra8875_readReg(uint8_t reg)
{
    uint8_t val;

    // ???????
    ra8875_select();
    spi_transfer(0x80);  // CMDWRITE
    spi_transfer(reg);
    ra8875_deselect();

    // ????
    ra8875_select();
    spi_transfer(0x40);  // DATAREAD
    val = spi_transfer(0x00); // dummy byte
    ra8875_deselect();

    return val;
}

