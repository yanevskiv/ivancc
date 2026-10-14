// (Test) Status: 0
// A test file for <stdarg.h> va_start after va_end.

// Include helpers.
#include "libc07_00_stdarg_helpers.inc"

// Two walks should agree: return the sum of count int arguments.
static int again(int count, ...)
{
    int second = 0;
    int first = 0;
    va_list ap;
    int i;

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        first += va_arg(ap, int);
    }
    va_end(ap);
    va_start(ap, count);
    for (i = 0; i < count; i++) {
        second += va_arg(ap, int);
    }
    va_end(ap);
    return first == second ? first : MISMATCH;
}

// [C99:7.15.1.4p3] va_start after va_end should walk the same arguments again.
static void Test_StdargAgain(void)
{
    _TEST_INT(again(0), 0);
    _TEST_INT(again(3, 1, 2, 3), 6);
    _TEST_INT(again(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
}

int main(void)
{
    Test_StdargAgain();
    return _Test_Status();
}
