// (Test) Status: 0
// A test file for <setjmp.h> longjmp given 0.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

// [C99 S7.13.2.1p4] setjmp should return 1 where longjmp was given 0.
static void Test_SetjmpZero(void)
{
    volatile int count = 0;

    if (setjmp(env) != 1) {
        once(&count, env, 2, 0);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            longjmp(env, 0);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_SetjmpZero();
    return _Test_Status();
}
