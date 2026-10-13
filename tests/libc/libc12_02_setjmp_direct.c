// (Test) Status: 0
// A test file for <setjmp.h> direct calls of setjmp.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

// [C99:S7.13.1.1p3] setjmp should return 0 from a direct call, whatever buffer.
static void Test_SetjmpDirect(void)
{
    jmp_buf many[BUFS];
    volatile int count = 0;
    struct Holder holder;

    if (setjmp(env) == 0) {
        count++;
    }
    _TEST_INT(count, 1);
    if (setjmp(many[2]) == 0) {
        count++;
    }
    _TEST_INT(count, 2);
    if (setjmp(holder.ho_env) == 0) {
        count++;
    }
    _TEST_INT(count, 3);
}

int main(void)
{
    Test_SetjmpDirect();
    return _Test_Status();
}
