#include <avr/io.h>
#include <util/delay.h>
#include "ra8875_drv.h"
#include "ra8875_gfx.h"
#include <stddef.h> 
#include <stdint.h>
#include <avr/pgmspace.h>
#include <stdio.h>   // ? sprintf

volatile uint8_t g_emotion_id = EMO_HAPPY;  // ?????????
/* ?????? */
#define RA8875_FGCR0  0x63 // R
#define RA8875_FGCR1  0x64 // G
#define RA8875_FGCR2  0x65 // B

/* ???? / ? / ???? */
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

#define CELL    25          // ????????
#define FACE_COLS 27        // ? 27 ?
#define FACE_ROWS 11        // ? 11 ?

#define MARGIN_LR   62      // ????
#define MARGIN_BOTTOM 40    // ????

// ???????
#define FACE_W_PIX  (FACE_COLS * CELL)          // 27*25 = 675
#define FACE_H_PIX  (FACE_ROWS * CELL)          // 11*25 = 275

#define FACE_X0     MARGIN_LR                   // = 62
#define FACE_Y0     (LCD_H - MARGIN_BOTTOM - FACE_H_PIX)
// = 480 - 62 - 275 = 143

#define FONT_W 5
#define FONT_H 7
//#define BIG_CELL 12  // ? ?????? 13x13 ??
//#define BIG8     7    // ????? T/W ? 8x8 ????
#define BIG11  8   // 11x11 的大点阵

//static uint16_t g_time_base_x = 0;
//static uint16_t g_time_base_y = 0;
//static uint8_t  g_time_layout_ready = 0;
//static uint16_t g_time_text_w = 0;   // ?? HH MM ??????
//
//// 新增：专门给冒号用的坐标
//static uint16_t g_time_colon_x = 0;
//static uint16_t g_time_colon_y = 0;
///* ---------- ?????????? ---------- */

static void ra8875_setFGColor(uint16_t color565)
{
    uint8_t r = ((color565 >> 11) & 0x1F) << 3;
    uint8_t g = ((color565 >> 5)  & 0x3F) << 2;
    uint8_t b = ( color565        & 0x1F) << 3;

    ra8875_writeReg(RA8875_FGCR0, r);
    ra8875_writeReg(RA8875_FGCR1, g);
    ra8875_writeReg(RA8875_FGCR2, b);
}

/* ---------- ?? ---------- */


// 用 11x11 方块画一个点（on=1 画前景色，on=0 画背景色）
static void big11_drawPixel(uint16_t x, uint16_t y,
                            uint16_t fg, uint16_t bg, uint8_t on)
{
    if (on) {
        ra8875_fillRect(x, y, BIG11, BIG11, fg);
    } else {
        ra8875_fillRect(x, y, BIG11, BIG11, bg);
    }
}



//void ra8875_drawPixel(uint16_t x, uint16_t y, uint16_t color)
//{
//    uint8_t hi = color >> 8;
//    uint8_t lo = color & 0xFF;
//
//    ra8875_setXY(x, y);
//    ra8875_writeCommand(RA8875_MRWC);
//
//    ra8875_writeData(hi);
//    ra8875_writeData(lo);
//}
//
//static void big13_drawPixel(uint16_t x, uint16_t y,
//                            uint16_t fg, uint16_t bg, uint8_t on)
//{
//    if (on) {
//        ra8875_fillRect(x, y, BIG_CELL, BIG_CELL, fg);
//    } else {
//        // ??????????????????
//        ra8875_fillRect(x, y, BIG_CELL, BIG_CELL, bg);
//    }
//}
//
//static void big8_drawPixel(uint16_t x, uint16_t y,
//                           uint16_t fg, uint16_t bg, uint8_t on)
//{
//    if (on) {
//        ra8875_fillRect(x, y, BIG8, BIG8, fg);
//    } else {
//        ra8875_fillRect(x, y, BIG8, BIG8, bg);
//    }
//}


/* ---------- ?? ---------- */
// ?????????????? ra8875_gfx.c ??static ????????
static void ra8875_wait_done(void)
{
    uint8_t v;
    do {
        v = ra8875_readReg(RA8875_DCR);
    } while (v & 0x80); // bit7=1 ?????
}

void ra8875_drawLine(uint16_t x0, uint16_t y0,
                     uint16_t x1, uint16_t y1,
                     uint16_t color)
{
    ra8875_setFGColor(color);

    // ??
    ra8875_writeReg(RA8875_DLHSR0, x0 & 0xFF);
    ra8875_writeReg(RA8875_DLHSR1, (x0 >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVSR0, y0 & 0xFF);
    ra8875_writeReg(RA8875_DLVSR1, (y0 >> 8) & 0x01);

    // ??
    ra8875_writeReg(RA8875_DLHER0, x1 & 0xFF);
    ra8875_writeReg(RA8875_DLHER1, (x1 >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVER0, y1 & 0xFF);
    ra8875_writeReg(RA8875_DLVER1, (y1 >> 8) & 0x01);

    // ????
    ra8875_writeReg(RA8875_DCR, 0x80);

    // ?????????
    ra8875_wait_done();
}


/* ---------- ???filled=0 ???? 0 ??? ---------- */

//void ra8875_drawCircle(uint16_t x, uint16_t y,
//                       uint16_t r,
//                       uint16_t color,
//                       uint8_t filled)
//{
//    ra8875_setFGColor(color);
//
//    // ??
//    ra8875_writeReg(RA8875_DCHR0, x & 0xFF);
//    ra8875_writeReg(RA8875_DCHR1, (x >> 8) & 0x03);
//    ra8875_writeReg(RA8875_DCVR0, y & 0xFF);
//    ra8875_writeReg(RA8875_DCVR1, (y >> 8) & 0x01);
//
//    // ??
//    ra8875_writeReg(RA8875_DCRR0, r & 0xFF);
//
//    uint8_t dcr = 0;
//    if (filled) dcr |= (1 << 5); // ??
//    dcr |= (1 << 6);             // ????
//
//    ra8875_writeReg(RA8875_DCR, dcr);
//
//    _delay_ms(2);
//}

void ra8875_fillSquare(uint16_t x, uint16_t y, uint16_t size, uint16_t color)
{
    uint16_t x0 = x;
    uint16_t y0 = y;
    uint16_t x1 = x + size;
    uint16_t y1 = y + size;

    ra8875_setFGColor(color);

    // ??????
    ra8875_writeReg(0x91, x0 & 0xFF);
    ra8875_writeReg(0x92, (x0 >> 8) & 0x03);
    ra8875_writeReg(0x93, y0 & 0xFF);
    ra8875_writeReg(0x94, (y0 >> 8) & 0x01);

    ra8875_writeReg(0x95, x1 & 0xFF);
    ra8875_writeReg(0x96, (x1 >> 8) & 0x03);
    ra8875_writeReg(0x97, y1 & 0xFF);
    ra8875_writeReg(0x98, (y1 >> 8) & 0x01);

    // ???????bit7=1 ?????bit5=1 ???
    ra8875_writeReg(0x90, 0xB0);

    // ??????
    uint8_t v;
    do {
        v = ra8875_readReg(0x90);
    } while (v & 0x80);
}

// (x,y) ?????w,h ???
void ra8875_fillRect(uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h,
                     uint16_t color)
{
    uint16_t x1 = x + w - 1;
    uint16_t y1 = y + h - 1;

    ra8875_setFGColor(color);

    // ????
    ra8875_writeReg(RA8875_MWCR0, 0x00);

    // ??
    ra8875_writeReg(RA8875_DLHSR0, x & 0xFF);
    ra8875_writeReg(RA8875_DLHSR1, (x >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVSR0, y & 0xFF);
    ra8875_writeReg(RA8875_DLVSR1, (y >> 8) & 0x01);

    // ??
    ra8875_writeReg(RA8875_DLHER0, x1 & 0xFF);
    ra8875_writeReg(RA8875_DLHER1, (x1 >> 8) & 0x03);
    ra8875_writeReg(RA8875_DLVER0, y1 & 0xFF);
    ra8875_writeReg(RA8875_DLVER1, (y1 >> 8) & 0x01);

    // DCR: bit7=1 ???bit5=1 ????
    ra8875_writeReg(RA8875_DCR, 0xB0);
    ra8875_wait_done();
}

// 11 ? × 27 ??1 = ???0 = ??
static const uint8_t face_bitmap_happier[FACE_ROWS][FACE_COLS] PROGMEM = {
    // 0 ??????
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    // 1 ?
    {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0},
    // 2 ??????
    {0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0},
    // 3 ??????
    {0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0},
    // 4 ??????
    {0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0},
    // 5 ????????
    {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0},
    // 6 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // 7 ??????
    {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
    // 8 ??????
    {0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0},
    // 9 ??????
    {0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0},
    // 10 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_lackWater[FACE_ROWS][FACE_COLS] PROGMEM = {
    // 0 ??????
    {0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0},
    // 1 ?
    {1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},
    // 2 ??????
    {1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1},
    // 3 ??????
    {1,1,0,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1},
    // 4 ??????
    {1,0,1,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,0,1},
    // 5 ????????
    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
    // 6 ??????
    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
    // 7 ??????
    {0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0},
    // 8 ??????
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
    // 9 ??????
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
    // 10 ??????
    {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_sleep[FACE_ROWS][FACE_COLS] PROGMEM = {
    // 0 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
    // 1 ?
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
    // 2 ??????
    {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0},
    // 3 ??????
    {0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,1,1,1},
    // 4 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    // 5 ????????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
    // 6 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},
    // 7 ??????
    {0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // 8 ??????
    {0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // 9 ??????
    {0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // 10 ??????
    {0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

//static const uint8_t face_bitmap_sleep1[FACE_ROWS][FACE_COLS] = {
//    // 0 ??????
//    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
//    // 1 ?
//    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
//    // 2 ??????
//    {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0},
//    // 3 ??????
//    {0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,1,1,1},
//    // 4 ??????
//    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
//    // 5 ????????
//    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
//    // 6 ??????
//    {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},
//    // 7 ??????
//    {0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//    // 8 ??????
//    {0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//    // 9 ??????
//    {0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//    // 10 ??????
//    {0,0,0,0,0,0,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
//};

static const uint8_t face_bitmap_hot[FACE_ROWS][FACE_COLS] PROGMEM = {
    // 0 ??????
    {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0},
    // 1 ?
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    // 2 ??????
    {1,0,1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0},
    // 3 ??????
    {0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0},
    // 4 ??????
    {0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,1},
    // 5 ????????
    {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
    // 6 ??????
    {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
    // 7 ??????
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // 8 ??????
    {1,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    // 9 ??????
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0},
    // 10 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_cold[FACE_ROWS][FACE_COLS] PROGMEM = {
    // 0 ??????
    {0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0},
    // 1 ?
    {0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1},
    // 2 ??????
    {0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0},
    // 3 ??????
    {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0},
    // 4 ??????
    {0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,0,0,0},
    // 5 ????????
    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
    // 6 ??????
    {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
    // 7 ??????
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    // 8 ??????
    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
    // 9 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
    // 10 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
};

static const uint8_t face_bitmap_normal[FACE_ROWS][FACE_COLS] PROGMEM = {
    // 0 ??????
    {0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0},
    // 1 ?
    {0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0},
    // 2 ??????
    {1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,0},
    // 3 ??????
    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
    // 4 ??????
    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
    // 5 ????????
    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
    // 6 ??????
    {0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0},
    // 7 ??????
    {0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0},
    // 8 ??????
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    // 9 ??????
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    // 10 ??????
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
};

// ??????????? bitmap ?????
static void draw_face_from_bitmap(const uint8_t bitmap[FACE_ROWS][FACE_COLS],
                                  uint16_t color)
{
    for (uint8_t row = 0; row < FACE_ROWS; row++) {
        uint8_t col = 0;
        while (col < FACE_COLS) {
            // ?? 0
            while (col < FACE_COLS && pgm_read_byte(&bitmap[row][col]) == 0) {
                col++;
            }
            if (col >= FACE_COLS) break;

            // ?????? 1
            uint8_t start = col;
            while (col < FACE_COLS && pgm_read_byte(&bitmap[row][col]) == 1) {
                col++;
            }
            uint8_t end = col - 1;

            // ???????????
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
    // ???????????????????????????
    ra8875_fillRect(FACE_X0, FACE_Y0,
                    FACE_W_PIX, FACE_H_PIX,
                    COLOR_BLACK);  // ???????

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
        // ???? ID????? happy
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

// ? ???????????? ?? ????????????????????
//void emotion_animate_step(uint16_t color)
//{
//    if (g_emotion_id == EMO_SLEEP) {
//        static uint8_t sleep_frame = 0;
//
//        // ???????? sleep / sleep1 ?????
//        if (sleep_frame == 0) {
//            draw_face_from_bitmap(face_bitmap_sleep, color);
//        } else {
//            draw_face_from_bitmap(face_bitmap_sleep1, color);
//        }
//        sleep_frame ^= 1;
//    }
//    // ?????????????
//}

//text


// 10 ??? 0?9???? 5 ???? 7 ????? 1 ????? 7 bit?




// 5x7 ???????
//static const uint8_t glyph_colon_5x7[FONT_W] = {
//    0x00,
//    0x36,   // ?? + ??
//    0x36,
//    0x00,
//    0x00
//};


//void gfx_draw_char_big13(uint16_t x, uint16_t y,
//                         char c, uint16_t fg, uint16_t bg)
//{
//    const uint8_t *glyph = NULL;
//
//    if (c >= '0' && c <= '9') {
//        glyph = font5x7_digits[(uint8_t)(c - '0')];
//    } else if (c >= 'A' && c <= 'Z') {
//        glyph = font5x7_letters[(uint8_t)(c - 'A')];
//    } else if (c >= 'a' && c <= 'z') {
//        glyph = font5x7_letters[(uint8_t)(c - 'a')];
//    } else if (c == ':') {
//        // ??? ':' ?????? 5x7 ??
//        static const uint8_t glyph_colon[FONT_W] = {
//            0x00,
//            0x36, // 00110110b??????
//            0x36, // 00110110b??????
//            0x00,
//            0x00
//        };
//        glyph = glyph_colon;
//    } else if (c == ' ') {
//        // ????????
//        for (uint8_t col = 0; col < FONT_W; col++) {
//            for (uint8_t row = 0; row < FONT_H; row++) {
//                uint16_t px = x + col * BIG_CELL;
//                uint16_t py = y + row * BIG_CELL;
//                big13_drawPixel(px, py, fg, bg, 0);
//            }
//        }
//        return;
//    } else {
//        // ???????????
//        return;
//    }
//
//    // ?? 5x7 ?????????? 13x13 ??
//    for (uint8_t col = 0; col < FONT_W; col++) {
//        uint8_t colBits = glyph[col];
//        for (uint8_t row = 0; row < FONT_H; row++) {
//            uint8_t on = (colBits & (1 << row)) ? 1 : 0;
//            uint16_t px = x + col * BIG_CELL;
//            uint16_t py = y + row * BIG_CELL;
//            big13_drawPixel(px, py, fg, bg, on);
//        }
//    }
//}
//
//void gfx_draw_char_big8(uint16_t x, uint16_t y,
//                        char c, uint16_t fg, uint16_t bg)
//{
//    const uint8_t *glyph = NULL;
//
//    if (c >= '0' && c <= '9') {
//        glyph = font5x7_digits[(uint8_t)(c - '0')];
//    } else if (c >= 'A' && c <= 'Z') {
//        glyph = font5x7_letters[(uint8_t)(c - 'A')];
//    } else if (c >= 'a' && c <= 'z') {
//        glyph = font5x7_letters[(uint8_t)(c - 'a')];
//    } else if (c == ':') {
//        static const uint8_t glyph_colon[FONT_W] = {
//            0x00,
//            0x36, // 00110110b??????
//            0x36, // 00110110b??????
//            0x00,
//            0x00
//        };
//        glyph = glyph_colon;
//    } else if (c == ' ') {
//        // ????????
//        for (uint8_t col = 0; col < FONT_W; col++) {
//            for (uint8_t row = 0; row < FONT_H; row++) {
//                uint16_t px = x + col * BIG8;
//                uint16_t py = y + row * BIG8;
//                big8_drawPixel(px, py, fg, bg, 0);
//            }
//        }
//        return;
//    } else {
//        // ???????????
//        return;
//    }
//
//    // ???????? 5x7 ??????? / ?? / ???
//    for (uint8_t col = 0; col < FONT_W; col++) {
//        uint8_t colBits = glyph[col];
//        for (uint8_t row = 0; row < FONT_H; row++) {
//            uint8_t on = (colBits & (1 << row)) ? 1 : 0;
//            uint16_t px = x + col * BIG8;
//            uint16_t py = y + row * BIG8;
//            big8_drawPixel(px, py, fg, bg, on);
//        }
//    }
//}
//
//
//void gfx_draw_string_big8(uint16_t x, uint16_t y,
//                          const char *s, uint16_t fg, uint16_t bg)
//{
//    uint16_t cursor_x = x;
//
//    while (*s) {
//        gfx_draw_char_big8(cursor_x, y, *s, fg, bg);
//        cursor_x += FONT_W * BIG8 + BIG8;  // ?????? BIG8
//        s++;
//    }
//}

//void gfx_draw_char(uint16_t x, uint16_t y, char c, uint16_t color)
//{
//    const uint8_t *glyph = NULL;
//
//    if (c >= '0' && c <= '9') {
//        glyph = font5x7_digits[(uint8_t)(c - '0')];
//    } else if (c >= 'A' && c <= 'Z') {
//        glyph = font5x7_letters[(uint8_t)(c - 'A')];
//    } else if (c >= 'a' && c <= 'z') {
//        // ?????????
//        glyph = font5x7_letters[(uint8_t)(c - 'a')];
//    } else if (c == ' ') {
//        // ???????
//        return;
//    } else {
//        // ????????
//        return;
//    }
//
//    for (uint8_t col = 0; col < FONT_W; col++) {
//        uint8_t colBits = glyph[col];  // ???? 7 ??
//        for (uint8_t row = 0; row < FONT_H; row++) {
//            if (colBits & (1 << row)) {    // ???????
//                ra8875_drawPixel(x + col, y + row, color);
//            }
//        }
//    }
//}
//
//
//void gfx_draw_string(uint16_t x, uint16_t y, const char *s, uint16_t color)
//{
//    uint16_t cursor_x = x;
//
//    while (*s) {
//        if (*s == '\n') {
//            // ??????? x????????????????+FONT_H+1?
//            y += FONT_H + 1;
//            cursor_x = x;
//        } else {
//            gfx_draw_char(cursor_x, y, *s, color);
//            cursor_x += FONT_W + 1;  // ???? 1 ??
//        }
//        s++;
//    }
//}

//void gfx_draw_string_big13(uint16_t x, uint16_t y,
//                           const char *s, uint16_t fg, uint16_t bg)
//{
//    uint16_t cursor_x = x;
//
//    while (*s) {
//        gfx_draw_char_big13(cursor_x, y, *s, fg, bg);
//        cursor_x += FONT_W * BIG_CELL + BIG_CELL;  // ??????? BIG_CELL ???
//        s++;
//    }
//}

                                  
//void gfx_draw_time_above_face_static(uint8_t hour,
//                                     uint8_t minute,
//                                     uint16_t fg, uint16_t bg)
//{
//    char buf[8];
//    // 现在直接画 "HH:MM" —— 中间是冒号
//    sprintf(buf, "%02u:%02u", hour, minute);
//
//    uint8_t len = 0;
//    while (buf[len] != '\0') len++;
//
//    // 和之前一样：一个大字符的水平步进 = 宽度 + 字符间距
//    uint16_t char_w = FONT_W * BIG_CELL + BIG_CELL;
//    uint16_t text_w = len * char_w - BIG_CELL;
//    uint16_t text_h = FONT_H * BIG_CELL;
//
//    // 纵向位置：在表情上方留一点空白
//    uint16_t y = (FACE_Y0 > text_h + 5) ? (FACE_Y0 - text_h - 5) : 0;
//    // 水平居中
//    uint16_t x = FACE_X0 + (FACE_W_PIX - text_w) / 2;
//
//    // 先清掉整条时间区域
//    ra8875_fillRect(FACE_X0, y, FACE_W_PIX, text_h, bg);
//
//    // 画整串 "HH:MM"
//    gfx_draw_string_big13(x, y, buf, fg, bg);
//
//    // 如果后面你不再需要冒号单独控制，这几个基准变量可要可不要
//    g_time_base_x = x;
//    g_time_base_y = y;
//    g_time_layout_ready = 1;
//}


//void gfx_update_time_colon(uint8_t second,
//                           uint16_t fg, uint16_t bg)
//{
//    if (!g_time_layout_ready) return;
//
//    // ✅ 不再重算 x/y，而是用画时间时存好的
//    uint16_t colon_x = g_time_colon_x;
//    uint16_t colon_y = g_time_colon_y;
//
//    // 冒号这一格的宽高：和一个大字符一样
//    uint16_t slot_w = FONT_W * BIG_CELL;
//    uint16_t slot_h = FONT_H * BIG_CELL;
//
//    // 先清掉这一块，避免残影
//    ra8875_fillRect(colon_x, colon_y, slot_w, slot_h, bg);
//
//    // 偶数秒：画冒号；奇数秒：保持空白
//    if ((second & 1) == 0) {
//        uint16_t dot_size = BIG_CELL;
//
//        // 水平居中
//        uint16_t dot_x = colon_x + (slot_w - dot_size) / 2;
//
//        // 让两个点在字符高度内上下对称一点
//        uint16_t mid_y  = colon_y + slot_h / 2;
//        uint16_t gap    = dot_size;  // 两点之间的距离“半径”
//
//        uint16_t dot_y1 = mid_y - gap - dot_size / 2;  // 上点
//        uint16_t dot_y2 = mid_y + gap - dot_size / 2;  // 下点
//
//        ra8875_fillRect(dot_x, dot_y1, dot_size, dot_size, fg);
//        ra8875_fillRect(dot_x, dot_y2, dot_size, dot_size, fg);
//    }
//}

//void gfx_draw_sensors(uint8_t temp,
//                      uint8_t water,
//                      uint16_t fg,
//                      uint16_t bg)
//{
//    if (!g_time_layout_ready) return;  // ?????????????? y?
//
//    char bufT[8];
//    char bufW[8];
//
//    // ??????T:25 / W:60
//    sprintf(bufT, "T %d", temp);
//    sprintf(bufW, "W %u", water);
//
//    // ???????
//    uint8_t lenT = 0, lenW = 0;
//    while (bufT[lenT] != '\0') lenT++;
//    while (bufW[lenW] != '\0') lenW++;
//
//    // ???????????5 ? + 1 ?????? BIG8 ??
//    uint16_t char_w8   = FONT_W * BIG8 + BIG8;
//    uint16_t text_w_T  = lenT * char_w8 - BIG8;
//    uint16_t text_w_W  = lenW * char_w8 - BIG8;
//    uint16_t text_h_8  = FONT_H * BIG8;
//
//    // ?? y_time ????????? y
//    uint16_t y_time = g_time_base_y;
//
//    uint16_t time_h = FONT_H * BIG_CELL;  // 91
//    uint16_t tw_h   = 8;                  // T/W ? 8x8 ????
//
//    // ? T/W ?????????
//    uint16_t y_TW = y_time + (time_h - tw_h)/2;
//
//    // ???????? 40 ??
//    const uint16_t margin = 40;
//
//    // ??????? 40
//    uint16_t xT = margin;
//
//    // ??????? 40?W:xx ???????? 40?
//    uint16_t xW;
//    if (LCD_W > margin + text_w_W) {
//        xW = LCD_W - margin - text_w_W;
//    } else {
//        // ?????????????????????
//        xW = 0;
//    }
//
//    // ???????????
//    ra8875_fillRect(xT, y_TW, text_w_T, text_h_8, bg);
//    ra8875_fillRect(xW, y_TW, text_w_W, text_h_8, bg);
//
//    // ?????
//    gfx_draw_string_big8(xT, y_TW, bufT, fg, bg);
//    gfx_draw_string_big8(xW, y_TW, bufW, fg, bg);
//}


// 一行显示：T XX  HH:MM  W XX
// 字体：11x11，小方块
// 距离屏幕上边缘 y=5，整行水平居中
/* ======= 极简 5x7 字库：只支持 0-9, T, W, :, 空格 ======= */

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

/* 单独为 'T' 和 'W' 做 5x7 字形（不用大字母表，避免乱七八糟） */
static const uint8_t glyph_T[FONT_W] = {
    0x01, 0x01, 0x7F, 0x01, 0x01
};

static const uint8_t glyph_W[FONT_W] = {
    0x7F, 0x20, 0x18, 0x20, 0x7F
};

/* 冒号 ':' 的 5x7 点阵 */
static const uint8_t glyph_colon[FONT_W] = {
    0x00,
    0x36,
    0x36,
    0x00,
    0x00
};

/* ======= 5x7 -> BIG11 放大的绘制函数 ======= */

void gfx_draw_char_big11(uint16_t x, uint16_t y,
                         char c, uint16_t fg, uint16_t bg)
{
    const uint8_t *glyph = NULL;

    if (c >= '0' && c <= '9') {
        glyph = font5x7_digits[(uint8_t)(c - '0')];
    } else if (c == 'T') {
        glyph = glyph_T;
    } else if (c == 'W') {
        glyph = glyph_W;
    } else if (c == ':') {
        glyph = glyph_colon;
    } else if (c == ' ') {
        /* 空格：整块画背景 */
        for (uint8_t col = 0; col < FONT_W; col++) {
            for (uint8_t row = 0; row < FONT_H; row++) {
                uint16_t px = x + col * BIG11;
                uint16_t py = y + row * BIG11;
                big11_drawPixel(px, py, fg, bg, 0);
            }
        }
        return;
    } else {
        /* 其他字符直接忽略，不画 */
        return;
    }

    /* 正常 5x7 放大为 8x8“小方块阵” */
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
        /* 每个字符占：5 列 * BIG11 + 1 个 BIG11 间距 */
        cursor_x += FONT_W * BIG11 + BIG11;
        s++;
    }
}

void time_init(uint8_t temp, uint8_t water,
               uint8_t hour, uint8_t minute,
               uint16_t fg, uint16_t bg)
{   
    char line[32];

    /* 拼成一行：T:25 12:34 W:60 */
    sprintf(line, "T:%d %02u:%02u W:%u", temp, hour, minute, water);

    /* 计算字符串长度 */
    uint8_t len = 0;
    while (line[len] != '\0') len++;

    /* 每个大字符的水平步进 */
    uint16_t char_w = FONT_W * BIG11 + BIG11;
    uint16_t text_w = len * char_w - BIG11;  // 最后一个字符后面不要多加间隔
    uint16_t text_h = FONT_H * BIG11;

    /* 顶部离上边界留 5 像素 */
    uint16_t y = 20;

    /* 水平居中 */
    uint16_t x = 0;
    if (LCD_W > text_w) {
        x = (LCD_W - text_w) / 2;
    }

    /* 先把整条区域清空（全屏宽，只占 text_h 高度） */
    ra8875_fillRect(0, y, LCD_W, text_h, bg);

    /* 再画一整行大字 */
    gfx_draw_string_big11(x, y, line, fg, bg);
    
}

void text_change(uint8_t temp, uint8_t water,
               uint8_t hour, uint8_t minute,
               uint16_t fg, uint16_t bg)
{   
    char line[32];

    /* 拼成一行：T:25 12:34 W:60 */
    sprintf(line, "T:%d %02u:%02u W:%u", temp, hour, minute, water);

    /* 计算字符串长度 */
    uint8_t len = 0;
    while (line[len] != '\0') len++;

    /* 每个大字符的水平步进 */
    uint16_t char_w = FONT_W * BIG11 + BIG11;
    uint16_t text_w = len * char_w - BIG11;  // 最后一个字符后面不要多加间隔
    uint16_t text_h = FONT_H * BIG11;

    /* 顶部离上边界留 5 像素 */
    uint16_t y = 20;

    /* 水平居中 */
    uint16_t x = 0;
    if (LCD_W > text_w) {
        x = (LCD_W - text_w) / 2;
    }

    /* 先把整条区域清空（全屏宽，只占 text_h 高度） */
    ra8875_fillRect(0, y, LCD_W, text_h, bg);

    /* 再画一整行大字 */
    gfx_draw_string_big11(x, y, line, fg, bg);
    
}
