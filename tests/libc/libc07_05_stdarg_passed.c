// (Test) Status: 0
// A test file for <stdarg.h> lists passed to functions.

// Include helpers.
#include "libc07_00_stdarg_helpers.inc"

#define BASE 10

// Return the sum of the count int arguments a list still holds.
static int walk(va_list ap, int count)
{
    int total = 0;

    while (count > 0) {
        total += va_arg(ap, int);
        count--;
    }
    return total;
}

// Return the sum of count int arguments, walked by another function.
static int forward(int count, ...)
{
    va_list ap;
    int total;

    va_start(ap, count);
    total = walk(ap, count);
    va_end(ap);
    return total;
}

// Return the next int argument of a list its owner walks on with.
static int take(va_list *ap)
{
    return va_arg(*ap, int);
}

// Return the int arguments as decimal digits, each other one taken apart.
static long shared(int count, ...)
{
    long digits = 0;
    va_list ap;
    int i;

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        digits = digits * BASE + (i % 2 == 0 ? take(&ap) : va_arg(ap, int));
    }
    va_end(ap);
    return digits;
}

// (S7.15p3) Another function should walk a list it is passed, by pointer too.
static void Test_StdargPassed(void)
{
    _TEST_INT(forward(0), 0);
    _TEST_INT(forward(3, 4, 5, 6), 15);
    _TEST_INT(forward(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
    _TEST_INT(shared(4, 1, 2, 3, 4), 1234);
    _TEST_INT(shared(9, 1, 2, 3, 4, 5, 6, 7, 8, 9), 123456789);
}

int main(void)
{
    Test_StdargPassed();
    return _Test_Status();
}
