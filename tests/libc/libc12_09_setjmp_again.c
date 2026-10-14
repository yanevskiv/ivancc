// (Test) Status: 0
// A test file for <setjmp.h> jump after jump to one buffer.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

// A buffer should take jump after jump, each with its value.
// [C99:7.13.2.1p2]: longjmp, restoring setjmp's environment
static void Test_SetjmpAgain(void)
{
    volatile int count = 0;

    while (setjmp(env) < 5) {
        each(&count, env, 3, 5);
        break;
    }
    _TEST_INT(count, 5);
}

int main(void)
{
    Test_SetjmpAgain();
    return _Test_Status();
}
