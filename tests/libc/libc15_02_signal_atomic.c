// (Test) Status: 0
// A test file for <signal.h> sig_atomic_t.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

// [C99/S7.14p2] A sig_atomic_t should hold each of its limits.
static void Test_SignalAtomic(void)
{
    volatile sig_atomic_t atom = SIG_ATOMIC_MAX;

    _TEST_INT(atom, SIG_ATOMIC_MAX);
    atom = SIG_ATOMIC_MIN;
    _TEST_INT(atom, SIG_ATOMIC_MIN);
    atom = 0;
    _TEST_INT(atom, 0);
}

int main(void)
{
    Test_SignalAtomic();
    return _Test_Status();
}
