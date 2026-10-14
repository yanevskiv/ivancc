// (Test) Status: 0
// A test file for <signal.h> handlers that leave by longjmp.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

static jmp_buf env;

// Note the signal and leave without returning to raise.
static void on_leave(int sig)
{
    got = sig;
    longjmp(env, 1);
}

// [C99:7.14.2.1p2] A handler that leaves by longjmp should not return to raise.
static void Test_SignalLeaves(void)
{
    volatile int returned = 0;

    got = 0;
    if (_TEST_TRUE(signal(SIGINT, on_leave) != SIG_ERR)) {
        if (setjmp(env) == 0) {
            raise(SIGINT);
            returned = 1;
        }
        _TEST_INT(got, SIGINT);
        _TEST_INT(returned, 0);
    }
}

int main(void)
{
    Test_SignalLeaves();
    return _Test_Status();
}
