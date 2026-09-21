// The north star: no kernel, no libc, nothing linked. The program is its own
// entry point, and everything it can do it does through the device registers
// ivanemu maps at 0x10000000.

int _start()
{
    char *uart;
    char *halt;
    char *msg;

    uart = (char *) 0x10000000;
    halt = (char *) 0x10000008;
    msg = "Hello world!\n";

    while (*msg) {
        *uart = *msg;
        msg = msg + 1;
    }

    *halt = 0;
    return 0;
}
