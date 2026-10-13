// (Test) Status: 0
// A test file for <signal.h> raise.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

// [C99/S7.14.2.1p2] raise should run a signal's handler once, with its number.
static bool raises(int sig)
{
    int lost = 0;

    got = 0;
    count = 0;
    if (! _TEST_TRUE(signal(sig, on_signal) != SIG_ERR)) {
        return false;
    }
    lost += ! _TEST_INT(raise(sig), 0);
    lost += ! _TEST_INT(got, sig);
    lost += ! _TEST_INT(count, 1);
    return lost == 0;
}

// [C99/S7.14.2.1p2] raise should run the handler of each signal C99 names.
static void Test_SignalRaises(void)
{
    _TEST_TRUE(raises(SIGABRT));
    _TEST_TRUE(raises(SIGFPE));
    _TEST_TRUE(raises(SIGILL));
    _TEST_TRUE(raises(SIGINT));
    _TEST_TRUE(raises(SIGSEGV));
    _TEST_TRUE(raises(SIGTERM));
}

int main(void)
{
    Test_SignalRaises();
    return _Test_Status();
}
