#include <avr/io.h>
#include <util/delay.h>
#include "ra8875_drv.h"
#include "ra8875_gfx.h"
#include <stddef.h> 
#include <stdint.h>
#include <avr/pgmspace.h>
#include <stdio.h>   

volatile uint8_t g_emotion_id = EMO_HAPPY;  
#define RA8875_FGCR0  0x63 // R
#define RA8875_FGCR1  0x64 // G
#define RA8875_FGCR2  0x65 // B

#define RA8875_DCR    0x90

#define RA8875_DLHSR0 0x91
#define RA8875_DLHSR1 0x92
#define RA8875_DLVSR0 0x93
#define RA8875_DLVSR1 0x94
#define RA8875_DLHER0 0x95
#define RA8875_DLHER1 0x96
#define RA8875_DLVER0 0x97
#define RA8875_DLVER1 0x98

#define RA8875_DCHR0  0x99
#define RA8875_DCHR1  0x9A
#define RA8875_DCVR0  0x9B
#define RA8875_DCVR1  0x9C
#define RA8875_DCRR0  0x9D

#define RA8875_MWCR0   0x40
#define RA8875_FGCR0   0x63
#define RA8875_FGCR1   0x64
#define RA8875_FGCR2   0x65

#define LCD_W   800
#define LCD_H   480

#define CELL    25          
#define FACE_COLS 27        
#define FACE_ROWS 11       

#define MARGIN_LR   62      
#define MARGIN_BOTTOM 40    


#define FACE_W_PIX  (FACE_COLS * CELL)          
#define FACE_H_PIX  (FACE_ROWS * CELL)         

#define FACE_X0     MARGIN_LR                  
#define FACE_Y0     (LCD_H - MARGIN_BOTTOM - FACE_H_PIX)


#define FONT_W 5
#define FONT_H 7

#define BIG11  8   

static void ra8875_setFGColor(uint16_t color565)
{
    uint8_t r = ((color565 >> 11) & 0x1F) << 3;
    uint8_t g = ((color565 >> 5)  & 0x3F) << 2;
    uint8_t b = ( color565        & 0x1F) << 3;

    ra8875_writeReg(RA8875_FGCR0, r);
    ra8875_writeReg(RA8875_FGCR1, g);
    ra8875_writeReg(RA8875_FGCR2, b);
}

static void big11_drawPixel(uint16_t x, uint16_t y,
                            uint16_t fg, uint16_t bg, uint8_t on)
{
    if (on) {
        ra8875_fillRect(x, y, BIG11, BIG11, fg);
    } else {
        ra8875_fillRect(x, y, BIG11, BIG11, bg);
    }
}

static void ra8875_wait_done(void)
{
    uint8_t v;
    do {
        v = ra8875_readReg(RA8875_DCR);
    } while (v & 0x80); 
}

void ra8875_drawLine(uint16_t x0, uint16_t y0,
                     uint16_t x1, uint16_t y1,
                     uint16_t color)
{
    ra8875_setFGColor(color);

    ra8875_writeReg(RA8875_DLHSR0, x0 & 0xFF);
    ra8875_writeReg(RA8875_DLHSR1, (x0 >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVSR0, y0 & 0xFF);
    ra8875_writeReg(RA8875_DLVSR1, (y0 >> 8) & 0x01);

    ra8875_writeReg(RA8875_DLHER0, x1 & 0xFF);
    ra8875_writeReg(RA8875_DLHER1, (x1 >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVER0, y1 & 0xFF);
    ra8875_writeReg(RA8875_DLVER1, (y1 >> 8) & 0x01);

    ra8875_writeReg(RA8875_DCR, 0x80);

    ra8875_wait_done();
}

void ra8875_fillSquare(uint16_t x, uint16_t y, uint16_t size, uint16_t color)
{
    uint16_t x0 = x;
    uint16_t y0 = y;
    uint16_t x1 = x + size;
    uint16_t y1 = y + size;

    ra8875_setFGColor(color);

    ra8875_writeReg(0x91, x0 & 0xFF);
    ra8875_writeReg(0x92, (x0 >> 8) & 0x03);
    ra8875_writeReg(0x93, y0 & 0xFF);
    ra8875_writeReg(0x94, (y0 >> 8) & 0x01);

    ra8875_writeReg(0x95, x1 & 0xFF);
    ra8875_writeReg(0x96, (x1 >> 8) & 0x03);
    ra8875_writeReg(0x97, y1 & 0xFF);
    ra8875_writeReg(0x98, (y1 >> 8) & 0x01);

    ra8875_writeReg(0x90, 0xB0);

    uint8_t v;
    do {
        v = ra8875_readReg(0x90);
    } while (v & 0x80);
}

void ra8875_fillRect(uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h,
                     uint16_t color)
{
    uint16_t x1 = x + w - 1;
    uint16_t y1 = y + h - 1;

    ra8875_setFGColor(color);

    ra8875_writeReg(RA8875_MWCR0, 0x00);

    ra8875_writeReg(RA8875_DLHSR0, x & 0xFF);
    ra8875_writeReg(RA8875_DLHSR1, (x >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVSR0, y & 0xFF);
    ra8875_writeReg(RA8875_DLVSR1, (y >> 8) & 0x01);

    ra8875_writeReg(RA8875_DLHER0, x1 & 0xFF);
    ra8875_writeReg(RA8875_DLHER1, (x1 >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVER0, y1 & 0xFF);
    ra8875_writeReg(RA8875_DLVER1, (y1 >> 8) & 0x01);

    ra8875_writeReg(RA8875_DCR, 0xB0);
    ra8875_wait_done();
}

static const uint8_t face_bitmap_happier[FACE_ROWS][FACE_COLS] PROGMEM = {

    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},

    {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0},

    {0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0},

    {0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0},

    {0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0},

    {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},

    {0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0},

    {0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_happier1[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0},

    {0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0},

    {0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0},

    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0},

    {0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},

    {0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0},

    {0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0},

    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_lackWater[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0},

    {1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},

    {1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},

    {1,1,0,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1},

    {1,0,1,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,0,1},

    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},

    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},

    {0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0},

    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_lackWater1[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0},

    {1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},

    {1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},

    {1,1,0,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1},

    {1,0,1,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,0,1},

    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},

    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},

    {0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0},

    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_sleep[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},

    {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0},

    {0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,1,1,1},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},

    {0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_sleep1[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},

    {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0},

    {0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,1,1,1},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},

    {0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_hot[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0},

    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},

    {1,0,1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0},

    {0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0},

    {0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,1},

    {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0},

    {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},

    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {1,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},

    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_hot1[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0},

    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},

    {1,0,1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0},

    {0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0},

    {0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0},

    {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,1},

    {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    {1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},

    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_cold[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0},

    {0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1},

    {0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0},

    {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0},

    {0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,0,0,0},

    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},

    {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_cold1[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0},

    {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0},

    {0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0},

    {0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1},

    {0,0,0,0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0},

    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},

    {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},

    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_normal[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0},

    {0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0},

    {1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,0},

    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0},

    {0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_normal1[FACE_ROWS][FACE_COLS] PROGMEM = {

    {0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0},

    {0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0},

    {1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,0},

    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

    {0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0},

    {0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0},

    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
};

static void draw_face_from_bitmap(const uint8_t bitmap[FACE_ROWS][FACE_COLS],
                                  uint16_t color)
{
    for (uint8_t row = 0; row < FACE_ROWS; row++) {
        uint8_t col = 0;
        while (col < FACE_COLS) {

            while (col < FACE_COLS && pgm_read_byte(&bitmap[row][col]) == 0) {
                col++;
            }
            if (col >= FACE_COLS) break;

            uint8_t start = col;
            while (col < FACE_COLS && pgm_read_byte(&bitmap[row][col]) == 1) {
                col++;
            }
            uint8_t end = col - 1;

            uint16_t x = FACE_X0 + start * CELL;
            uint16_t y = FACE_Y0 + row   * CELL;
            uint16_t w = (end - start + 1) * CELL;
            uint16_t h = CELL;

            ra8875_fillRect(x, y, w, h, color);
        }
    }
}

void draw_face_by_id(int mood, uint16_t color)
{
    ra8875_fillRect(FACE_X0, FACE_Y0,
                    FACE_W_PIX, FACE_H_PIX,
                    COLOR_BLACK); 

    switch (mood) {
    case 0:
        draw_face_from_bitmap(face_bitmap_sleep, color);
        break;
    case 1:
        draw_face_from_bitmap(face_bitmap_happier, color);
        break;

    case 2:
        draw_face_from_bitmap(face_bitmap_hot, color);
        break;
        
    case 3:
        draw_face_from_bitmap(face_bitmap_cold, color);
        break;
        
    case 4:
        draw_face_from_bitmap(face_bitmap_lackWater, color);
        break;
        
    case 5:
        draw_face_from_bitmap(face_bitmap_normal, color);
        break;
        
    default:
        draw_face_from_bitmap(face_bitmap_normal, color);
        break;
    }
}

void emotion_set(int mood, uint16_t color)
{
    draw_face_by_id(mood, color);
}

void emotion_init(int mood, uint16_t color)
{
    draw_face_by_id(mood, color);
}

static void draw_face_diff(const uint8_t from[FACE_ROWS][FACE_COLS],
                           const uint8_t to  [FACE_ROWS][FACE_COLS])
{
    for (uint8_t row = 0; row < FACE_ROWS; row++) {
        for (uint8_t col = 0; col < FACE_COLS; col++) {

            uint8_t a = pgm_read_byte(&from[row][col]);
            uint8_t b = pgm_read_byte(&to  [row][col]);

            if (a == b) {
                continue;
            }

            uint16_t x = FACE_X0 + col * CELL;
            uint16_t y = FACE_Y0 + row * CELL;

            if (b == 1) {
                ra8875_fillRect(x, y, CELL, CELL, COLOR_WHITE);
            } else {
                ra8875_fillRect(x, y, CELL, CELL, COLOR_BLACK);
            }
        }
    }
}


void emotion_animate_step(int mood)
{
    static uint8_t switchpoint = 0;
    switch (mood) {
    case 0:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_sleep, face_bitmap_sleep1);
        } else {
            draw_face_diff(face_bitmap_sleep1, face_bitmap_sleep);
        }
        switchpoint ^= 1;
        break;
    case 1:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_happier, face_bitmap_happier1);            
        } else {
            draw_face_diff(face_bitmap_happier1, face_bitmap_happier); 
        }
        switchpoint ^= 1;
        break;

    case 2:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_hot, face_bitmap_hot1);             
        } else {
            draw_face_diff(face_bitmap_hot1, face_bitmap_hot);   
        }
        switchpoint ^= 1;
        break;
        
    case 3:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_cold, face_bitmap_cold1);
        } else {
            draw_face_diff(face_bitmap_cold1, face_bitmap_cold);
        }
        switchpoint ^= 1;
        break;
        
    case 4:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_lackWater, face_bitmap_lackWater1);               
        } else {
            draw_face_diff(face_bitmap_lackWater1, face_bitmap_lackWater);
        }
        switchpoint ^= 1;
        break;
        
    case 5:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_normal, face_bitmap_normal1);            
        } else {
            draw_face_diff(face_bitmap_normal1, face_bitmap_normal);  
        }
        switchpoint ^= 1;
        break;
        
    default:
        if (switchpoint == 0) {
            draw_face_diff(face_bitmap_normal, face_bitmap_normal1);            
        } else {
            draw_face_diff(face_bitmap_normal1, face_bitmap_normal);  
        }
        switchpoint ^= 1;
        break;
    }
}

static const uint8_t font5x7_digits[10][FONT_W] = {
    // '0'
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // '1'
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // '2'
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // '3'
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // '4'
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // '5'
    {0x27, 0x45, 0x45, 0x45, 0x39},
    // '6'
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    // '7'
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // '8'
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // '9'
    {0x06, 0x49, 0x49, 0x29, 0x1E},
};

static const uint8_t glyph_T[FONT_W] = {
    0x01, 0x01, 0x7F, 0x01, 0x01
};

static const uint8_t glyph_H[FONT_W] = {
    0x7F, 0x08, 0x08, 0x08, 0x7F
};

static const uint8_t glyph_colon[FONT_W] = {
    0x00,
    0x36,
    0x36,
    0x00,
    0x00
};

void gfx_draw_char_big11(uint16_t x, uint16_t y,
                         char c, uint16_t fg, uint16_t bg)
{
    const uint8_t *glyph = NULL;

    if (c >= '0' && c <= '9') {
        glyph = font5x7_digits[(uint8_t)(c - '0')];
    } else if (c == 'T') {
        glyph = glyph_T;
    } else if (c == 'H') {
        glyph = glyph_H;
    } else if (c == ':') {
        glyph = glyph_colon;
    } else if (c == ' ') {
        for (uint8_t col = 0; col < FONT_W; col++) {
            for (uint8_t row = 0; row < FONT_H; row++) {
                uint16_t px = x + col * BIG11;
                uint16_t py = y + row * BIG11;
                big11_drawPixel(px, py, fg, bg, 0);
            }
        }
        return;
    } else {
        return;
    }

    for (uint8_t col = 0; col < FONT_W; col++) {
        uint8_t colBits = glyph[col];
        for (uint8_t row = 0; row < FONT_H; row++) {
            uint8_t on = (colBits & (1 << row)) ? 1 : 0;
            uint16_t px = x + col * BIG11;
            uint16_t py = y + row * BIG11;
            big11_drawPixel(px, py, fg, bg, on);
        }
    }
}

void gfx_draw_string_big11(uint16_t x, uint16_t y,
                           const char *s, uint16_t fg, uint16_t bg)
{
    uint16_t cursor_x = x;

    while (*s) {
        gfx_draw_char_big11(cursor_x, y, *s, fg, bg);
        cursor_x += FONT_W * BIG11 + BIG11;
        s++;
    }
}

void time_init(uint8_t temp, uint8_t water,
               uint8_t hour, uint8_t minute,
               uint16_t fg, uint16_t bg)
{   
    draw_temp_water_side(temp, water, COLOR_YELLOW, COLOR_BLACK);
    draw_time_center(hour, minute, COLOR_YELLOW, COLOR_BLACK);
  
}

static uint8_t str_len(const char *s)
{
    uint8_t len = 0;
    while (s[len] != '\0') len++;
    return len;
}

#define TEXT_Y    20
#define SIDE_MARGIN  10   

void draw_time_center(uint8_t hour, uint8_t minute,
                      uint16_t fg, uint16_t bg)
{
    char line[16];
    sprintf(line, "%02u:%02u", hour, minute);

    uint8_t len = str_len(line);

    uint16_t char_w = FONT_W * BIG11 + BIG11;
    uint16_t text_w = len * char_w - BIG11;  
    uint16_t text_h = FONT_H * BIG11;

    uint16_t y = TEXT_Y;

    uint16_t x = 0;
    if (LCD_W > text_w) {
        x = (LCD_W - text_w) / 2;
    }

    ra8875_fillRect(x, y, text_w, text_h, bg);

    gfx_draw_string_big11(x, y, line, fg, bg);
}

void toggle_colon(uint16_t fg, uint16_t bg)
{
    static uint8_t colon_on = 1;   

    uint8_t len = 5;   
    uint16_t char_w = FONT_W * BIG11 + BIG11;
    uint16_t text_w = len * char_w - BIG11;
    uint16_t text_h = FONT_H * BIG11;
    uint16_t y = TEXT_Y;

    uint16_t x = 0;
    if (LCD_W > text_w) {
        x = (LCD_W - text_w) / 2;
    }

    uint16_t colon_x = x + 2 * char_w;

    ra8875_fillRect(colon_x, y, char_w, text_h, bg);

    if (colon_on) {
        gfx_draw_string_big11(colon_x, y, ":", fg, bg);
    }

    colon_on = !colon_on;  
}

void draw_temp_water_side(uint8_t temp, uint8_t water,
                          uint16_t fg, uint16_t bg)
{
    char left[16];
    char right[16];

    sprintf(left,  "T:%u", (unsigned)temp);
    sprintf(right, "H:%u", (unsigned)water);

    uint16_t char_w = FONT_W * BIG11 + BIG11;
    uint16_t text_h = FONT_H * BIG11;
    uint16_t y = TEXT_Y;

    uint8_t lenL = str_len(left);
    uint16_t text_wL = lenL * char_w - BIG11;
    uint16_t xL = SIDE_MARGIN;

    uint8_t lenR = str_len(right);
    uint16_t text_wR = lenR * char_w - BIG11;
    uint16_t xR = LCD_W - SIDE_MARGIN - text_wR;

    ra8875_fillRect(xL, y, text_wL, text_h, bg);
    ra8875_fillRect(xR, y, text_wR, text_h, bg);

    gfx_draw_string_big11(xL, y, left,  fg, bg);
    gfx_draw_string_big11(xR, y, right, fg, bg);
}


