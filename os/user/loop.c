static const char msg[] = "loop process running\n";

void _start() {
    while (1) {
        __asm__ volatile ("nop");
    }
}
