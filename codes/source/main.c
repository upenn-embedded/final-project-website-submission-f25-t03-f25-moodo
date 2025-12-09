#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#include "uart.h"
#include "sht40.h"
#include "ds1307.h"
#include "twi0.h"
#include "sound.h"

#include "ra8875_drv.h"
#include "ra8875_gfx.h"


#include <avr/interrupt.h>

// -----------------------------------------------------------------------------
// Global state
// -----------------------------------------------------------------------------

// Mood is kept for future display / eye expression, but is NOT used to
// trigger any sounds. Sounds depend only on distance, humidity and light.
int      mood  = 0;      // 0=sleep,1=happy,2=hot,3=cold,4=thirsty,5=normal
int      last_mood = 0;  // 0=sleep,1=happy,2=hot,3=cold,4=thirsty,5=normal
uint16_t light = 0;      // photoresistor ADC reading (PC0 / ADC0)
int      textcode = 0;   // 1=text change; 0= text keep

// Environment data from SHT40
typedef struct {
    float    temp_c;
    float    temp_c_pre;
    float    rh_percent;
    float    rh_percent_pre;
    uint16_t minute_pre;
    uint16_t hour_pre;
    uint16_t second_pre;
    uint16_t raw_t;
    uint16_t raw_rh;
    uint8_t  valid;      // 1 if last read was successful
} env_t;

static env_t g_env = {0};

// -----------------------------------------------------------------------------
// Ultrasonic distance measurement using Timer1 Input Capture
//  - Echo on PB0 / ICP1
//  - Trig on PD3
// -----------------------------------------------------------------------------

volatile uint16_t icr_start = 0, icr_end = 0;
volatile uint8_t  have_pulse = 0, rising = 1;

ISR(TIMER1_CAPT_vect)
{
    uint16_t v = ICR1;

    if (rising) {
        // first edge: rising, record start time
        icr_start = v;
        TCCR1B &= ~(1 << ICES1);   // next capture on falling edge
        rising = 0;
    } else {
        // second edge: falling, record end time
        icr_end   = v;
        have_pulse = 1;
        TCCR1B |= (1 << ICES1);    // next capture on rising edge
        rising = 1;
    }
}

static void icp_init(void)
{
    DDRB  &= ~(1 << DDB0);                  // PB0 as input (echo)
    TCCR1A = 0;
    // Noise canceler, capture on rising edge, prescaler = 8
    TCCR1B = (1 << ICNC1) | (1 << ICES1) | (1 << CS11);
    TCCR1C = 0;
    TIMSK1 = (1 << ICIE1);                  // enable input capture interrupt
}

static void trig_10us(void)
{
    DDRD  |= (1 << PD3);        // PD3 as output (new trig pin)
    PORTD &= ~(1 << PD3);
    _delay_us(2);
    PORTD |=  (1 << PD3);
    _delay_us(12);
    PORTD &= ~(1 << PD3);
}


// -----------------------------------------------------------------------------
// ADC for photoresistor (PC0 / ADC0)
// -----------------------------------------------------------------------------

static void adc_init(void)
{
    ADMUX  = (1 << REFS0);                  // AVcc reference, ADC0
    ADCSRA = (1 << ADEN) |                  // enable ADC
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
    DIDR0  = (1 << ADC0D);                  // disable digital input on ADC0
}

static uint16_t adc_read(void)
{
    ADCSRA |= (1 << ADSC);                  // start conversion
    while (ADCSRA & (1 << ADSC)) {
        ;                                   // wait until conversion completes
    }
    return ADC;
}

// -----------------------------------------------------------------------------
// Read environment from SHT40 once per loop and print all values.
//
// This function:
//   - updates g_env.temp_c / g_env.rh_percent / g_env.raw_t / g_env.raw_rh
//   - sets g_env.valid = 1 on success, 0 on error
//   - prints raw and converted values to UART
// -----------------------------------------------------------------------------

static void update_environment(void)
{
    float    t, rh;
    uint16_t raw_t, raw_rh;

    if (SHT40_read(&t, &rh) == 0) {
        // If you also want explicit raw values, call SHT40_read_raw
        if (SHT40_read_raw(&raw_t, &raw_rh) == 0) {
            g_env.raw_t  = raw_t;
            g_env.raw_rh = raw_rh;
        } else {
            g_env.raw_t  = 0;
            g_env.raw_rh = 0;
        }

        g_env.temp_c    = t;
        g_env.rh_percent = rh;
        g_env.valid     = 1;

        printf("ENV: T_raw=%u, RH_raw=%u\r\n", g_env.raw_t, g_env.raw_rh);
        printf("ENV: T=%.2f C, RH=%.2f %%\r\n",
               g_env.temp_c, g_env.rh_percent);
    } else {
        g_env.valid = 0;
        printf("ENV: ERROR reading SHT40\r\n");
    }
}

// -----------------------------------------------------------------------------
// Light measurement and light-related sounds.
//
// Rules:
//   - Threshold at light_adc = 250
//       bright: light >= 250
//       dark  : light < 250
//   - bright -> dark transition  : play SOUND_LIGHT_OFF (T03.wav)
//   - dark   -> bright transition: play SOUND_LIGHT_ON  (T04.wav)
//   - dark for ~10 consecutive loops (?10 s) : play SOUND_SLEEP (T01.wav)
// -----------------------------------------------------------------------------

static void update_light_and_sounds(void)
{
    static uint16_t prev_light = 0;
    static uint8_t  first      = 1;
    static uint16_t dark_seconds  = 0;
    static uint8_t  sleep_played  = 0;

    uint16_t current = adc_read();
    light = current;

    uint8_t was_dark = (!first && (prev_light < 250));
    uint8_t is_dark  = (current   < 250);

    printf("LIGHT: adc=%u (prev=%u) -> %s\r\n",
           current, prev_light, is_dark ? "DARK" : "BRIGHT");

    // LIGHT_OFF / LIGHT_ON on threshold crossing
    if (!first && (is_dark != was_dark)) {
        if (is_dark) {
            printf("SOUND: LIGHT_OFF (bright -> dark)\r\n");
            sound_play(SOUND_LIGHT_OFF);
        } else {
            printf("SOUND: LIGHT_ON (dark -> bright)\r\n");
            sound_play(SOUND_LIGHT_ON);
            // allow SLEEP to be triggered again after coming back to dark
            sleep_played = 0;
        }
    }

    // Count how long we stay in dark; after ~10 s play SLEEP once
    if (is_dark) {
        if (dark_seconds < 0xFFFF) {
            dark_seconds++;
        }
        if (!sleep_played && dark_seconds >= 10) {
            printf("SOUND: SLEEP (dark for ~10s)\r\n");
            sound_play(SOUND_SLEEP);
            sleep_played = 1;
        }
    } else {
        dark_seconds = 0;
    }

    prev_light = current;
    first      = 0;
}

// -----------------------------------------------------------------------------
// Humidity-related sound (THIRSTY).
//
// Rule:
//   - let dry = (RH < 25%)
//   - when RH crosses from >=25% to <25% (not every loop), play THIRSTY.
// -----------------------------------------------------------------------------

static void update_humidity_sound(void)
{
    static uint8_t was_dry = 0;

    if (!g_env.valid) {
        printf("HUM: env invalid, skip THIRSTY check\r\n");
        return;
    }

    uint8_t is_dry = (g_env.rh_percent < 25.0f);

    printf("HUM: RH=%.2f %%  is_dry=%u, was_dry=%u\r\n",
           g_env.rh_percent, is_dry, was_dry);

    if (is_dry && !was_dry) {
        printf("SOUND: THIRSTY (RH < 25%% threshold crossed)\r\n");
        sound_play(SOUND_THIRSTY);
    }

    was_dry = is_dry;
}

// -----------------------------------------------------------------------------
// Decide mood (for display/debug only, NOT for sounds).
// Uses current light and environment readings.
// -----------------------------------------------------------------------------

static void decide_mood(void)
{
    if (!g_env.valid) {
        printf("MOOD: cannot update (env invalid)\r\n");
        return;
    }

    int old = mood;

    if (light < 400) {//250
        mood = 0;     // sleep
    } else if ((g_env.temp_c >= 24.0f) && (g_env.temp_c <= 25.0f) &&
               (g_env.rh_percent >= 60.0f) && (g_env.rh_percent <= 90.0f)) {//rh<70
        mood = 1;     // happy
    } else if (g_env.temp_c >= 26.0f) {//t>35
        mood = 2;     // hot
    } else if (g_env.temp_c <= 22.0f) {//t<5
        mood = 3;     // cold
    } else if (g_env.rh_percent < 45.0f) { //rh<10
        mood = 4;     // thirsty (logical label only, no sound here)
    } else {
        mood = 5;     // normal
    }

    printf("MOOD: old=%d new=%d\r\n", old, mood);
}

//emotion update
static void update_emotion_display(void)
{
    if (mood != last_mood) {
        emotion_set(mood, COLOR_WHITE);  //change
    }
    last_mood = mood;
}

static void emotion_move(void)
{
    emotion_animate_step(mood);
}


// -----------------------------------------------------------------------------
// Distance measurement + distance-related sounds.
//
// Timer1 clock: 16MHz / 8 -> 0.5 us per tick
//   delta_ticks = icr_end - icr_start
//   time_us     = delta_ticks * 0.5
//   distance_cm = time_us * 0.0343 * 0.5   (speed of sound & round trip)
//
// Rules:
//   near = (distance_cm < 40 cm)
//
//   - At power-up: play HELLO once.
//   - far -> near transition          : play HELLO (T00.wav)
//   - near for ~60 consecutive loops  : play STAND_UP (T05.wav)
//     (reminder to stand up / move)
// -----------------------------------------------------------------------------

static void detect_distance_and_sounds(void)
{
    const uint16_t TIMEOUT_TICKS         = 60000;   // ~30 ms @ 16MHz/8
    const float    TICK_US               = 0.5f;
    const float    SOUND_SPEED_CM_PER_US = 0.0343f;
    const float    ROUND_TRIP_DIV2       = 0.5f;

    // With ~1 s main-loop delay, 60 loops ~ 60 seconds
    const uint16_t NEAR_THRESHOLD_LOOPS  = 10;
    const uint16_t FAR_THRESHOLD_LOOPS   = 2;

    // Persistent state across calls
    static uint8_t  last_near        = 0;
    static uint8_t  session_active   = 0;   // set after user has been near at least once
    static uint16_t near_loops       = 0;
    static uint16_t far_loops        = 0;
    static uint8_t  standup_played   = 0;
    static uint8_t  sleep_played     = 0;

    float   time_us     = 0.0f;
    float   distance_cm = 0.0f;
    uint8_t now_near    = 0;
    uint8_t now_far     = 0;

    // Prepare for new measurement
    have_pulse = 0;
    rising     = 1;
    TCNT1      = 0;

    trig_10us();

    while (!have_pulse && (TCNT1 < TIMEOUT_TICKS)) {
        // wait for echo or timeout
    }

    if (have_pulse) {
        uint16_t delta = (icr_end >= icr_start)
                         ? (icr_end - icr_start)
                         : (uint16_t)(icr_end + (uint16_t)(0xFFFF - icr_start) + 1);

        time_us     = delta * TICK_US;
        distance_cm = time_us * SOUND_SPEED_CM_PER_US * ROUND_TRIP_DIV2;

        now_near = (distance_cm < 40.0f);
        now_far  = (distance_cm > 70.0f);

        printf("DIST: ticks=%u, time=%.1f us, dist=%.1f cm, now_near=%u, now_far=%u\r\n",
               delta, time_us, distance_cm, now_near, now_far);
    } else {
        // No echo: treat as far
        now_near = 0;
        now_far  = 1;
        printf("DIST: timeout / no echo -> treat as FAR\r\n");
    }

    printf("DIST: before update: last_near=%u, session_active=%u, near_loops=%u, far_loops=%u, standup_played=%u, sleep_played=%u\r\n",
           last_near, session_active, near_loops, far_loops, standup_played, sleep_played);

    // HELLO on far -> near transition (or first time near after power-up)
    if (now_near && !last_near) {
        printf("SOUND: HELLO (person just came close)\r\n");
        sound_play(SOUND_HELLO);
        session_active = 1;      // we are now in an "interaction session"
    }

    if (now_near) {
        // User is near: accumulate near_loops
        if (near_loops < 0xFFFF) {
            near_loops++;
        }

        if (!standup_played && near_loops >= NEAR_THRESHOLD_LOOPS) {
            printf("SOUND: STAND_UP (near for a long time)\r\n");
            sound_play(SOUND_STAND_UP);
            standup_played = 1;
        }

        // While user is near, "away" state is cleared
        far_loops    = 0;
        sleep_played = 0;
    } else {
        // Not near: reset near-related counters
        near_loops     = 0;
        standup_played = 0;

        // Only consider SLEEP if we had an active session before
        if (session_active && now_far) {
            if (far_loops < 0xFFFF) {
                far_loops++;
            }

            if (!sleep_played && far_loops >= FAR_THRESHOLD_LOOPS) {
                printf("SOUND: SLEEP (far for a long time after being near)\r\n");
                sound_play(SOUND_SLEEP);
                sleep_played   = 1;
                session_active = 0;   // end this session; next HELLO will start a new one
            }
        } else {
            // Either no session yet, or mid-range (40?70 cm)
            far_loops = 0;
        }
    }

    last_near = now_near;

    printf("DIST: after update: session_active=%u, near_loops=%u, far_loops=%u\r\n",
           session_active, near_loops, far_loops);
}

// -----------------------------------------------------------------------------
// RTC printing helper
// -----------------------------------------------------------------------------

static void real_time_clock(void)
{
    ds1307_time_t t;

    if (DS1307_read_time(&t) == 0) {
        printf("RTC: %02u:%02u:%02u  %02u/%02u/20%02u\r\n",
               t.hour, t.minute, t.second,
               t.month, t.date, t.year);
        

        //if(g_env.temp_c_pre != g_env.temp_c || g_env.rh_percent_pre != g_env.rh_percent || g_env.minute_pre != t.minute || g_env.hour_pre != t.hour)
        if(g_env.minute_pre != t.minute || g_env.hour_pre != t.hour){
            draw_time_center(t.hour, t.minute,
                        COLOR_YELLOW, COLOR_BLACK);
            
            g_env.minute_pre = t.minute;
            g_env.hour_pre = t.hour;
        }
        
        if(g_env.second_pre != t.second){
            toggle_colon(COLOR_YELLOW, COLOR_BLACK);
            g_env.second_pre = t.second;
        }
        
        if((int)g_env.temp_c_pre != (int)g_env.temp_c || (int)g_env.rh_percent_pre != (int)g_env.rh_percent){
            draw_temp_water_side(g_env.temp_c, g_env.rh_percent,
                            COLOR_YELLOW, COLOR_BLACK);
            
            g_env.temp_c_pre = g_env.temp_c;
            g_env.rh_percent_pre = g_env.rh_percent;
        }
        
            
    } else {
        printf("RTC: read ERROR\r\n");
    }
    
    
}

// -----------------------------------------------------------------------------
// Pump control (PD7)
// -----------------------------------------------------------------------------
void pump_init(void) {
    
    DDRD |= (1 << PD7);

    PORTD &= ~(1 << PD7);
}

void pump_on(void) {
    PORTD |= (1 << PD7);  
}

void pump_off(void) {
    PORTD &= ~(1 << PD7);  
}

void pump_scheduler(void)
{
    static uint16_t second_counter = 0;

    // Count seconds (because main loop = ~1 second)
    second_counter++;

    if (second_counter == 1) {
        pump_on();
        printf("PUMP: ON\n");
    }

    if (second_counter == 4) {   // after 5 seconds (1..5)
        pump_off();
        printf("PUMP: OFF\n");
    }

    if (second_counter >= 20) {  // reset every minute
        second_counter = 0;
    }

}   

// -----------------------------------------------------------------------------
// Global initialisation
// -----------------------------------------------------------------------------


static void init(void)
{

    
    // On-board LED on PB5
    DDRB |= (1 << PB5);

    // UART for logging
    uart_init();
    printf("\r\n=== E-Plant Demo: Light + Humidity + Distance + Sound ===\r\n");

    // Audio FX sound triggers (T00?T05)
    sound_init();

    // SHT40 on TWI0 (PC4 = SDA0, PC5 = SCL0)
    SHT40_init();
    printf("INFO: SHT40/TWI0 initialised.\r\n");

    // DS1307 RTC
    DS1307_init();
    printf("INFO: DS1307 RTC initialised.\r\n");

    // Pump control (PD7)
    pump_init();

    printf("INFO: SPI/LCD init...\r\n");
    
    //Emotion and text 
    spi_init();
    ra8875_init_800x480();
    ra8875_fillScreen(COLOR_BLACK);
    
    emotion_set(5,COLOR_WHITE);
    time_init(25, 60, 12, 34, COLOR_YELLOW, COLOR_BLACK);


    // (Optional) set a known initial time; remove this when RTC already keeps time
    ds1307_time_t t;
    t.second = 00;
    t.minute = 20;
    t.hour   = 13;
    t.day    = 5;
    t.date   = 21;
    t.month  = 11;
    t.year   = 25;
    
    if (DS1307_set_time(&t) == 0) {
        printf("INFO: RTC time set to 18:37:50 11/18/2025.\r\n");
    }
    
    //initial environment data
    g_env.temp_c_pre = 0;
    g_env.rh_percent_pre = 0;

    //initial time for text change
    g_env.minute_pre = 32;
    g_env.hour_pre = 14;
    g_env.second_pre = 50;
    
    // ADC for photoresistor
    adc_init();

    // Ultrasonic sensor on Timer1 input capture
    
    icp_init();
    

    // Enable global interrupts for Timer1 capture
    sei();

    // Timer0 PWM on PD5 (speaker tone, not directly used by sound module)
    DDRD  |= (1 << PD5);
    TCCR0A = (1 << WGM00) | (1 << COM0B1);
    TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
    OCR0A  = 119;                 // ~1046 Hz
    OCR0B  = OCR0A / 2;           // ~50% duty cycle
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(void)
{
    init();

    // Power-up greeting on T00.wav
    printf("SOUND: HELLO (power-up greeting)\r\n");
    sound_play(SOUND_HELLO);

    while (1) {
        // 1) Read SHT40 once and print all values
        update_environment();

        // 2) Measure light level and handle LIGHT_ON / LIGHT_OFF / SLEEP
        update_light_and_sounds();

        // 3) Use current light + environment to compute mood (for debug only)
        decide_mood();
        update_emotion_display();
        emotion_move();

        // 4) Check humidity and trigger THIRSTY if it becomes very dry
        update_humidity_sound();

        // 5) Measure distance and handle HELLO / STAND_UP
        detect_distance_and_sounds();

        // 6) Print current time from RTC
        real_time_clock();

        // 7) Pump control scheduler
        pump_scheduler();

        // Blink on-board LED so we can see the main loop is alive conflict with SPI
        //PORTB ^= (1 << PB5);
        
        //text update

        _delay_ms(1000);          // main loop period ~1 s
    }
}