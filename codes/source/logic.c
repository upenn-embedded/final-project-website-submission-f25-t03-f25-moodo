#include <avr/io.h>
#include <stdint.h>
#include "ra8875_gfx.h"

static uint8_t s_hour        = 0;
static uint8_t s_minute      = 0;

static int8_t  s_last_temp   = 0;
static uint8_t s_last_water  = 0;

static uint8_t s_time_drawn    = 0;
static uint8_t s_sensors_drawn = 0;
static uint8_t s_last_emo      = 0xFF;

void logic_init(void)
{
    s_time_drawn    = 0;
    s_sensors_drawn = 0;
    s_last_emo      = 0xFF;

    // 画初始表情
    emotion_init();
}

void logic_update(uint8_t hour,
                  uint8_t minute,
                  uint8_t second,
                  int8_t  temp,
                  uint8_t water,
                  uint8_t mood)
{
    (void)second; // 现在不用 second 了，避免编译器 warning

    // ---------- 1) 时间：只有小时/分钟变才刷新 ----------
    if (!s_time_drawn || hour != s_hour || minute != s_minute) {
        gfx_draw_time_above_face_static(hour, minute,
                                        COLOR_YELLOW, COLOR_BLACK);
        s_hour   = hour;
        s_minute = minute;
        s_time_drawn = 1;
    }

    // ---------- 2) 传感器显示 ----------
    if (!s_sensors_drawn ||
        temp  != s_last_temp ||
        water != s_last_water) {

        gfx_draw_sensors(temp, water,
                         COLOR_YELLOW, COLOR_BLACK);

        s_last_temp    = temp;
        s_last_water   = water;
        s_sensors_drawn = 1;
    }

    // ---------- 3) 用 mood 控制表情 ----------
    uint8_t emo_id;

    switch (mood) {
        case 0: emo_id = EMO_SLEEP; break;     // sleep
        case 1: emo_id = EMO_HAPPY; break;     // happy
        case 2: emo_id = EMO_HOT;   break;     // hot
        case 3: emo_id = EMO_COLD;  break;     // cold
        case 4: emo_id = EMO_SAD;   break;     // thirsty -> 用 sad 代替
        case 5: emo_id = EMO_HAPPY; break;     // normal -> happy
        default: emo_id = EMO_HAPPY; break;
    }

    if (emo_id != s_last_emo) {
        emotion_set(emo_id);
        s_last_emo = emo_id;
    }

    // 如果你不想 sleep 有动画，可以删掉下面这段
    /*
    if (emo_id == EMO_SLEEP) {
        emotion_animate_step(COLOR_WHITE);
    }
    */
}
