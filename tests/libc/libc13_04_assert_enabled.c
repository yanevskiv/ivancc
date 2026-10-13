// (Test) Status: 0
// A test file for <assert.h> assert included again without NDEBUG.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

#define NDEBUG
#include <assert.h>

#undef assert

#undef NDEBUG
#include <assert.h>

// [C99 S7.2p1] Included again without NDEBUG, assert should evaluate once more.
static void Test_AssertEnabled(void)
{
    count = 0;
    assert(bump() == 1);
    assert(bump() == 2);
    _TEST_INT(count, 2);
    assert(count = TEN);
    _TEST_INT(count, TEN);
}

int main(void)
{
    Test_AssertEnabled();
    return _Test_Status();
}
