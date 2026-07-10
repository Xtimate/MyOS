#include "driver.h"
#include "rtl8139.h"
#include "../include/framebuffer.h"

static driver_t drivers[] = {
    { "rtl8139", rtl8139_init },

};

#define DRIVER_COUNT (sizeof(drivers) / sizeof(drivers[0]))

void drivers_init_all() {
    for (unsigned int i = 0; i < DRIVER_COUNT; i++) {
        fb_terminal_print("driver: ");
        fb_terminal_print(drivers[i].name);
        fb_terminal_print("\n");

        if (drivers[i].init()) {
            fb_terminal_print(" ok\n");
        } else {
            fb_terminal_print(" not found\n");
        }
    }
}
