// (Test) Status: 0
// A test file for <stdarg.h> va_copy.

// Include helpers.
#include "libc07_00_stdarg_helpers.inc"

// A list and its copy should agree: return the sum of the ints past the first.
static int rest(int count, ...)
{
    int copied = 0;
    int total = 0;
    va_list copy;
    va_list ap;
    int i;

    va_start(ap, count);
    (void) va_arg(ap, int);
    va_copy(copy, ap);
    for (i = 1; i < count; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);
    for (i = 1; i < count; i++) {
        copied += va_arg(copy, int);
    }
    va_end(copy);
    return copied == total ? total : MISMATCH;
}

// [C99 S7.15.1.2p2] va_copy's list should stand where its source does, and apart.
static void Test_StdargCopy(void)
{
    _TEST_INT(rest(1, 1), 0);
    _TEST_INT(rest(4, 1, 2, 3, 4), 9);
    _TEST_INT(rest(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 54);
}

int main(void)
{
    Test_StdargCopy();
    return _Test_Status();
}
