// (Test) Status: 0
// A test file for <assert.h> assert without NDEBUG.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

// [C99/S7.2.1.1p2] Without NDEBUG, assert should evaluate its argument once.
static void Test_AssertActive(void)
{
    count = 0;
    assert(bump() == 1);
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_AssertActive();
    return _Test_Status();
}
