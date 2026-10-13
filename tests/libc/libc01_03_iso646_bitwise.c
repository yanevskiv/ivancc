// (Test) Status: 0
// A test file for <iso646.h> bitwise operators.

// Include helpers.
#include "libc01_00_iso646_helpers.inc"

// [C99:S7.9p1] bitand, bitor, xor and compl should work on their operands' bits.
static void Test_Iso646Bitwise(void)
{
    _TEST_INT(SIX bitand THREE, 2);
    _TEST_INT(SIX bitor THREE, 7);
    _TEST_INT(SIX xor THREE, 5);
    _TEST_INT(compl 0, -1);
    _TEST_INT(compl SIX bitand THREE, 1);
    _TEST_INT(SIX xor THREE bitand 1, 7);
}

int main(void)
{
    Test_Iso646Bitwise();
    return _Test_Status();
}
