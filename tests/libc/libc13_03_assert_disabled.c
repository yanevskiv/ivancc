// (Test) Status: 0
// A test file for <assert.h> assert under NDEBUG.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

#define NDEBUG
#include <assert.h>

// [C99:7.2p1] Under NDEBUG, assert should evaluate nothing, an expression still.
static void Test_AssertDisabled(void)
{
    count = 0;
    assert(0);
    assert(bump() == 0);
    assert(count = TEN);
    _TEST_INT(count, 0);
    _TEST_INT((assert(0), FIVE), FIVE);
}

int main(void)
{
    Test_AssertDisabled();
    return _Test_Status();
}
