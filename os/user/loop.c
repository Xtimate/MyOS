#include <stdio.h>

int main(int argc, char **argv) {
    while (1) {
        printf("loop\n");
        for (volatile int i = 0; i < 500000000; i++);
    }
    return 0;
}
