#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _start() {
    printf("hello from newlib!\n");

    // test malloc
    char *buf = malloc(64);
    strcpy(buf, "malloc works!");
    printf("%s\n", buf);
    free(buf);

    exit(0);
}
