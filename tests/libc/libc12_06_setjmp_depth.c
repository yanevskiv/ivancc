// (Test) Status: 0
// A test file for <setjmp.h> jumps from many calls down.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

#define DEEP    20
#define ROTATED 7654321

// Jump from depth calls down, each passing a seventh argument on the stack.
static long spill(jmp_buf to, int depth, long a, long b, long c, long d, long e, long f, long g)
{
    if (depth == 0) {
        leap(to, (int) (a + b * 10 + c * 100 + d * 1000 + e * 10000 + f * 100000 + g * 1000000));
        return 0;
    }
    return spill(to, depth - 1, g, a, b, c, d, e, f) + 1;
}

// Return the sum of seven arguments, the last passed on the stack.
static long add(long a, long b, long c, long d, long e, long f, long g)
{
    return a + b + c + d + e + f + g;
}

// [C99 S7.13.2.1p2] longjmp should return from many calls down, past the stack.
static void Test_SetjmpDepth(void)
{
    volatile int count = 0;

    if (setjmp(env) == 0) {
        once(&count, env, DEEP, 7);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != ROTATED) {
        count++;
        if (count == 1) {
            spill(env, DEEP + 1, 1, 2, 3, 4, 5, 6, 7);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
    _TEST_INT(add(1, 2, 3, 4, 5, 6, 7), 28);
}

int main(void)
{
    Test_SetjmpDepth();
    return _Test_Status();
}
