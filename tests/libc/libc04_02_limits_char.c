// (Test) Status: 0
// A test file for <limits.h> character limits.

// Include helpers.
#include "libc04_00_limits_helpers.inc"

// [C99/S5.2.4.2.1p2] A character should have 8 bits, and char its type's limits.
static void Test_LimitsChar(void)
{
    _TEST_INT(CHAR_BIT, 8);
    _TEST_INT(SCHAR_MIN, -128);
    _TEST_INT(SCHAR_MAX, 127);
    _TEST_INT(UCHAR_MAX, 255);
    _TEST_INT(UCHAR_MAX, (1 << CHAR_BIT) - 1);
    _TEST_INT(CHAR_MIN, -128);
    _TEST_INT(CHAR_MAX, 127);
    _TEST_INT(CHAR_MIN, (char) -1 < 0 ? SCHAR_MIN : 0);
    _TEST_INT(CHAR_MAX, (char) -1 < 0 ? SCHAR_MAX : UCHAR_MAX);
    _TEST_INT(MB_LEN_MAX, 16);
}

int main(void)
{
    Test_LimitsChar();
    return _Test_Status();
}
