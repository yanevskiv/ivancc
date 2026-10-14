// (Test) Status: 134
// (Test) Error:
// | a.out: libc13_06_assert_fails.c:12: fail: Assertion `str[0] == '"' || strcmp(str, "a\\b\"c") == 0' failed.
// A test file for <assert.h> a false assertion.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

// Assert a string opens with a quote or is one with a quote and a backslash.
static void fail(const char *str)
{
    assert(str[0] == '"' || strcmp(str, "a\\b\"c") == 0);
}

// A false argument should report itself and end the program.
// [C99:7.2.1.1p2]: assert's message and abort
static void Test_AssertFails(void)
{
    fail("\"");
    fail("a\\b\"c");
    fail("b");
}

int main(void)
{
    Test_AssertFails();
    return _Test_Status();
}
