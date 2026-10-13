// (Test) Status: 0
// A test file for <setjmp.h> values longjmp gives setjmp.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

// [C99/S7.13.2.1p4] setjmp should return the value longjmp was given, if not 0.
static void Test_SetjmpValues(void)
{
    volatile int count = 0;

    switch (setjmp(env)) {
    case 0:
        once(&count, env, 2, 42);
        break;
    case 42:
        count += LANDED;
        break;
    default:
        break;
    }
    _TEST_INT(count, 1 + LANDED);

    count = 0;
    if (setjmp(env) != 1) {
        once(&count, env, 2, 1);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != -1) {
        once(&count, env, 2, -1);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (-1 != setjmp(env)) {
        once(&count, env, 2, -1);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != INT_MAX) {
        once(&count, env, 2, INT_MAX);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != INT_MIN) {
        once(&count, env, 2, INT_MIN);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) < 0) {
        count += LANDED;
    } else {
        once(&count, env, 2, -7);
    }
    _TEST_INT(count, 1 + LANDED);
}

int main(void)
{
    Test_SetjmpValues();
    return _Test_Status();
}
