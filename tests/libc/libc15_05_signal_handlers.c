// (Test) Status: 0
// A test file for <signal.h> handlers raise runs.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

#define TWICE 2

// Install this handler again, as one that runs twice must where it is reset.
static void on_reinstall(int sig)
{
    signal(sig, on_reinstall);
    count++;
}

// [C99/S7.14.1.1p2] raise should run the handler last installed, none if ignored.
static void Test_SignalHandlers(void)
{
    got = 0;
    if (_TEST_TRUE(signal(SIGTERM, on_other) != SIG_ERR)) {
        _TEST_INT(raise(SIGTERM), 0);
        _TEST_INT(got, -SIGTERM);
    }

    got = 0;
    if (_TEST_TRUE(signal(SIGTERM, SIG_IGN) != SIG_ERR)) {
        _TEST_INT(raise(SIGTERM), 0);
        _TEST_INT(got, 0);
    }
    _TEST_TRUE(signal(SIGTERM, SIG_DFL) == SIG_IGN);

    count = 0;
    if (_TEST_TRUE(signal(SIGFPE, on_reinstall) != SIG_ERR)) {
        _TEST_INT(raise(SIGFPE), 0);
        _TEST_INT(raise(SIGFPE), 0);
        _TEST_INT(count, TWICE);
    }
}

int main(void)
{
    Test_SignalHandlers();
    return _Test_Status();
}
