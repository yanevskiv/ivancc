// (Test) Status: 0
// A test file for <signal.h> numbers signal refuses.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

// signal should refuse a number, and say why in errno.
// [C99:7.14.1.1p8]: signal's previous handler, or SIG_ERR and a positive errno
static bool refuses(int sig)
{
    int lost = 0;
    int number;
    void (*handler)(int);

    errno = 0;
    handler = signal(sig, on_signal);
    number = errno;
    lost += ! _TEST_TRUE(handler == SIG_ERR);
    lost += ! _TEST_TRUE(number > 0);
    return lost == 0;
}

// signal should refuse a number that is no signal's.
// [C99:7.14.1.1p8]: signal's previous handler, or SIG_ERR and a positive errno
static void Test_SignalRefused(void)
{
    _TEST_TRUE(refuses(-1));
    _TEST_TRUE(refuses(0));
    _TEST_TRUE(refuses(INT_MAX));
}

int main(void)
{
    Test_SignalRefused();
    return _Test_Status();
}
