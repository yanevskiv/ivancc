// (Test) Status: 0
// A test file for <setjmp.h> calls by pointer and past a macro.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

// [C99:S7.1.4p1] A pointer and a call in parentheses should reach longjmp.
static void Test_SetjmpCalls(void)
{
    volatile int count = 0;
    void (*jumper)(jmp_buf, int) = longjmp;

    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            jumper(env, 3);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            (longjmp)(env, 3);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_SetjmpCalls();
    return _Test_Status();
}
