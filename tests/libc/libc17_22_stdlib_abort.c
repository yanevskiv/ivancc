// (Test) Status: 0
// A test file for <stdlib.h> abort.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

static volatile sig_atomic_t aborted;
static jmp_buf env;

// A handler of SIGABRT should be free to leave abort.
// [C99:7.20.4.1p2]: abort, unless SIGABRT is caught
static void on_abort(int sig)
{
    aborted = sig == SIGABRT;
    longjmp(env, 1);
}

// abort should raise SIGABRT, whose handler may leave it.
// [C99:7.20.4.1p2]: abort, unless SIGABRT is caught
static void Test_StdlibAbort(void)
{
    volatile int returned = 0;

    aborted = 0;
    if (_TEST_TRUE(signal(SIGABRT, on_abort) != SIG_ERR)) {
        if (setjmp(env) == 0) {
            abort();
            returned = 1;
        }
        _TEST_INT(aborted, 1);
        _TEST_INT(returned, 0);
        _TEST_TRUE(signal(SIGABRT, SIG_DFL) != SIG_ERR);
    }
}

int main(void)
{
    Test_StdlibAbort();
    return _Test_Status();
}
