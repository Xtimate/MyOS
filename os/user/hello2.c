#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    printf("hello from newlib!\n");
    char *buf = malloc(64);
    strcpy(buf, "malloc works!");
    printf("%s\n", buf);
    free(buf);
    return 0;
}
