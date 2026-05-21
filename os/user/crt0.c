void _exit(int status);
extern int main(int argc, char **argv);

void _start(int argc, char **argv) {
    int ret = main(argc, argv);
    _exit(ret);
}
