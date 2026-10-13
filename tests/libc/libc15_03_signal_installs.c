// (Test) Status: 0
// A test file for <signal.h> handlers signal replaces.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

// [C99/S7.14.1.1p8] signal should return the handler it replaces.
static void Test_SignalInstalls(void)
{
    void (*first)(int) = signal(SIGTERM, on_signal);

    _TEST_TRUE(first == SIG_DFL || first == SIG_IGN);
    _TEST_TRUE(signal(SIGTERM, on_other) == on_signal);
    _TEST_TRUE(signal(SIGTERM, SIG_DFL) == on_other);
    _TEST_TRUE(signal(SIGTERM, first) == SIG_DFL);
    _TEST_TRUE(signal(SIGTERM, first) == first);
}

int main(void)
{
    Test_SignalInstalls();
    return _Test_Status();
}
