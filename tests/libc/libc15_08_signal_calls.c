// (Test) Status: 0
// A test file for <signal.h> calls by pointer and past a macro.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

// (S7.1.4p1) A pointer and a call in parentheses should reach each function.
static void Test_SignalCalls(void)
{
    int (*send)(int) = raise;
    void (*before)(int);
    void (*(*set)(int, void (*)(int)))(int) = signal;

    got = 0;
    if (_TEST_TRUE(set(SIGILL, on_signal) != SIG_ERR)) {
        _TEST_INT(send(SIGILL), 0);
        _TEST_INT(got, SIGILL);
    }
    before = (signal)(SIGILL, on_other);
    if (_TEST_TRUE(before == on_signal || before == SIG_DFL)) {
        _TEST_INT((raise)(SIGILL), 0);
        _TEST_INT(got, -SIGILL);
    }
}

int main(void)
{
    Test_SignalCalls();
    return _Test_Status();
}
