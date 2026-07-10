#ifndef DRIVER_H
#define DRIVER_H

typedef struct {
    const char *name;
    int (*init)(void);
} driver_t;


void drivers_init_all();

#endif
