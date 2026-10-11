// (Test) Status: 0
// A test file for <stdbool.h> bool conversions.

// Include helpers.
#include "libc02_00_stdbool_helpers.inc"

#define LARGE 256

// (S6.3.1.2p1) A value unequal to 0 should convert to 1, and one equal to 0.
static void Test_StdboolConversion(void)
{
    bool zero = 0;
    bool none = 0.0;
    bool null = NULL;
    bool large = LARGE;
    bool number = 42;
    bool negative = -1;
    bool fraction = 0.5;
    bool address = &zero;

    _TEST_INT(zero, 0);
    _TEST_INT(none, 0);
    _TEST_INT(null, 0);
    _TEST_INT(large, 1);
    _TEST_INT(number, 1);
    _TEST_INT(negative, 1);
    _TEST_INT(fraction, 1);
    _TEST_INT(address, 1);
}

int main(void)
{
    Test_StdboolConversion();
    return _Test_Status();
}
