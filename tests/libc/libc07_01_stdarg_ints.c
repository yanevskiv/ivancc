// (Test) Status: 0
// A test file for <stdarg.h> int arguments.

// Include helpers.
#include "libc07_00_stdarg_helpers.inc"

// Return the sum of count int arguments.
static int sum(int count, ...)
{
    int total = 0;
    va_list ap;

    va_start(ap, count);
    while (count > 0) {
        total += va_arg(ap, int);
        count--;
    }
    va_end(ap);
    return total;
}

// [C99:7.15.1.1p2] va_arg should yield each int in turn, in registers or not.
static void Test_StdargInts(void)
{
    _TEST_INT(sum(0), 0);
    _TEST_INT(sum(1, SEVEN), SEVEN);
    _TEST_INT(sum(3, 1, 2, 3), 6);
    _TEST_INT(sum(5, 1, 2, 3, 4, 5), 15);
    _TEST_INT(sum(6, 1, 2, 3, 4, 5, 6), 21);
    _TEST_INT(sum(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
    _TEST_INT(sum(3, INT_MAX, INT_MIN, -1), -2);
}

int main(void)
{
    Test_StdargInts();
    return _Test_Status();
}
