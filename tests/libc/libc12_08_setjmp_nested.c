// (Test) Status: 0
// A test file for <setjmp.h> buffers saved within another's reach.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

#define STRAYED 100
#define BASE    10

static jmp_buf outer;

// A jump from five calls down should land here once: return how many times.
static int inner(void)
{
    jmp_buf local;
    volatile int count = 0;

    if (setjmp(local) == 0) {
        once(&count, local, 5, 9);
    }
    return count;
}

// [C99:7.13.2.1p2] A jump should land at its own buffer's setjmp, among others.
static void Test_SetjmpNested(void)
{
    jmp_buf local;
    volatile int step = 0;
    volatile int count = 0;
    volatile int nested = 0;

    if (setjmp(outer) == 0) {
        step = step * BASE + 1;
        if (setjmp(local) == 0) {
            step = step * BASE + 2;
            once(&nested, local, 4, 1);
        }
        step = step * BASE + 3;
        _TEST_INT(inner(), 1);
        step = step * BASE + 4;
        once(&count, outer, 4, 1);
    }
    _TEST_INT(step, 1234);
    _TEST_INT(nested, 1);
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(outer) == 0) {
        if (setjmp(env) == 0) {
            once(&count, outer, 2, 1);
        }
        count += STRAYED;
    }
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_SetjmpNested();
    return _Test_Status();
}
