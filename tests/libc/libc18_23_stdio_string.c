// (Test) Status: 0
// A test file for <stdio.h> the s conversion.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.1p8] %s should write a string to its null or its precision's bytes.
static void Test_StdioString(void)
{
    const char open[3] = {'x', 'y', 'z'};

    _TEST_TRUE(prints("abc", "%s", "abc"));
    _TEST_TRUE(prints("", "%s", ""));
    _TEST_TRUE(prints("[abc][def]", "[%s][%s]", "abc", "def"));
    _TEST_TRUE(prints("%d", "%s", "%d"));

    _TEST_TRUE(prints("ab", "%.2s", "abc"));
    _TEST_TRUE(prints("abc", "%.3s", "abc"));
    _TEST_TRUE(prints("abc", "%.20s", "abc"));
    _TEST_TRUE(prints("", "%.0s", "abc"));
    _TEST_TRUE(prints("", "%.s", "abc"));
    _TEST_TRUE(prints("a", "%.01s", "abc"));

    _TEST_TRUE(prints("xyz", "%.3s", open));
    _TEST_TRUE(prints("x", "%.1s", open));
}

int main(void)
{
    Test_StdioString();
    return _Test_Status();
}
