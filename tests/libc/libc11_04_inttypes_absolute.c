// (Test) Status: 0
// A test file for <inttypes.h> imaxabs.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99 S7.8.2.1p2] imaxabs should give a number without its sign.
static void Test_InttypesAbsolute(void)
{
    _TEST_INT(imaxabs(0), 0);
    _TEST_INT(imaxabs(-5), 5);
    _TEST_INT(imaxabs(5), 5);
    _TEST_INT(imaxabs(INTMAX_MAX), INTMAX_MAX);
    _TEST_INT(imaxabs(-INTMAX_MAX), INTMAX_MAX);
}

int main(void)
{
    Test_InttypesAbsolute();
    return _Test_Status();
}
