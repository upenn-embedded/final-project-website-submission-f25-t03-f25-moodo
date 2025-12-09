#ifndef RA8875_GFX_H
#define RA8875_GFX_H

#include <stdint.h>

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0

#define LCD_W   800
#define LCD_H   480

#define EMO_HAPPY  1
#define EMO_SAD    2
#define EMO_SLEEP  3
#define EMO_HOT    4
#define EMO_COLD   5


extern volatile uint8_t g_emotion_id;

void ra8875_drawPixel(uint16_t x, uint16_t y, uint16_t color);
void ra8875_drawLine(uint16_t x0, uint16_t y0,
                     uint16_t x1, uint16_t y1,
                     uint16_t color);
void ra8875_drawCircle(uint16_t x, uint16_t y,
                       uint16_t r,
                       uint16_t color,
                       uint8_t filled);
void ra8875_fillSquare(uint16_t x, uint16_t y,
                       uint16_t size, uint16_t color);
void ra8875_fillRect(uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h,
                     uint16_t color);
void emotion_set(int mood, uint16_t color);
void emotion_init(int mood, uint16_t color);

void gfx_draw_char_big11(uint16_t x, uint16_t y,
                         char c, uint16_t fg, uint16_t bg);
void gfx_draw_string_big11(uint16_t x, uint16_t y,
                           const char *s, uint16_t fg, uint16_t bg);

void time_init(uint8_t temp, uint8_t water,
               uint8_t hour, uint8_t minute,
               uint16_t fg, uint16_t bg);
void draw_time_center(uint8_t hour, uint8_t minute,
                      uint16_t fg, uint16_t bg);
void toggle_colon(uint16_t fg, uint16_t bg);

void draw_temp_water_side(uint8_t temp, uint8_t water,
                          uint16_t fg, uint16_t bg);

void draw_face_by_id(int mood, uint16_t color);
void emotion_animate_step(int mood);

#endif 
