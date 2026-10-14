// (Test) Status: 0
// A test file for <signal.h> macros.

// Include helpers.
#include "libc15_00_signal_helpers.inc"

#define NONE    (-1)
#define SIGNALS 6

// The signals C99 names.
static const int sigs[SIGNALS] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};

// [C99:7.14p3] Each signal should be positive: return the first that is not.
static int nonpositive(void)
{
    int i;

    for (i = 0; i < SIGNALS; i++) {
        if (sigs[i] <= 0) {
            return i;
        }
    }
    return NONE;
}

// [C99:7.14p3] The signals should be distinct: return the first to repeat.
static int repeated(void)
{
    int i;
    int j;

    for (i = 0; i < SIGNALS; i++) {
        for (j = 0; j < i; j++) {
            if (sigs[i] == sigs[j]) {
                return i;
            }
        }
    }
    return NONE;
}

// [C99:7.14p3] The signals should be distinct, the three handlers no function's.
static void Test_SignalMacros(void)
{
    _TEST_INT(nonpositive(), NONE);
    _TEST_INT(repeated(), NONE);
    _TEST_UINT(sizeof(SIGABRT), sizeof(int));
    _TEST_TRUE(SIG_DFL != SIG_ERR);
    _TEST_TRUE(SIG_DFL != SIG_IGN);
    _TEST_TRUE(SIG_ERR != SIG_IGN);
    _TEST_TRUE(SIG_DFL != on_signal);
    _TEST_TRUE(SIG_ERR != on_signal);
    _TEST_TRUE(SIG_IGN != on_signal);
}

int main(void)
{
    Test_SignalMacros();
    return _Test_Status();
}
