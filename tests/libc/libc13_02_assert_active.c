// (Test) Status: 0
// A test file for <assert.h> assert without NDEBUG.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

// Without NDEBUG, assert should evaluate its argument once.
// [C99:7.2.1.1p2]: assert's message and abort
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
