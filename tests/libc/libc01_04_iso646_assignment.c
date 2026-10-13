// (Test) Status: 0
// A test file for <iso646.h> assignment operators.

// Include helpers.
#include "libc01_00_iso646_helpers.inc"

// [C99:S7.9p1] and_eq, or_eq and xor_eq should store their result and yield it.
static void Test_Iso646Assignment(void)
{
    int value = SIX;

    value and_eq THREE;
    _TEST_INT(value, 2);
    value or_eq 5;
    _TEST_INT(value, 7);
    value xor_eq 2;
    _TEST_INT(value, 5);
    _TEST_INT(value and_eq 4, 4);
    _TEST_INT(value or_eq 1, 5);
    _TEST_INT(value xor_eq 5, 0);
    _TEST_INT(value, 0);
}

int main(void)
{
    Test_Iso646Assignment();
    return _Test_Status();
}
