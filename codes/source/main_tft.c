// main.c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "ra8875_drv.h"
#include "ra8875_gfx.h"
#include "logic.h"

int main(void)
{
    spi_init();
    ra8875_init_800x480();

    ra8875_fillScreen(COLOR_BLACK);

    // ?????????? logic.c ??????
    logic_run();

    // ????????
    while (1) { }
}
