// (Test) Status: 0
// A test file for <setjmp.h> contexts of a setjmp call.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

// [C99 S7.13.1.1p4] setjmp should take a jump in each context C99 allows it.
static void Test_SetjmpContexts(void)
{
    volatile int count = 0;

    if (setjmp(env)) {
        count += LANDED;
    } else {
        once(&count, env, 1, 3);
    }
    _TEST_INT(count, 1 + LANDED);

    count = 0;
    while (! setjmp(env)) {
        once(&count, env, 1, 3);
        break;
    }
    _TEST_INT(count, 1);

    count = 0;
    for (; setjmp(env) <= 2; ) {
        each(&count, env, 1, 3);
        break;
    }
    _TEST_INT(count, 3);

    count = 0;
    do {
        count++;
        if (count == 2) {
            jump(env, 1, 8);
            count = RETURNED;
        }
        if (count != 1) {
            break;
        }
    } while (setjmp(env) == 0);
    _TEST_INT(count, 2);

    count = 0;
    if (2 > setjmp(env)) {
        once(&count, env, 1, 2);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) >= 5) {
        count += LANDED;
    } else {
        once(&count, env, 1, 5);
    }
    _TEST_INT(count, 1 + LANDED);

    count = 0;
    if (0 == setjmp(env)) {
        once(&count, env, 1, 4);
    }
    _TEST_INT(count, 1);

    count = 0;
    setjmp(env);
    once(&count, env, 1, 6);
    _TEST_INT(count, 2);

    count = 0;
    (void) setjmp(env);
    once(&count, env, 1, 6);
    _TEST_INT(count, 2);
}

int main(void)
{
    Test_SetjmpContexts();
    return _Test_Status();
}
