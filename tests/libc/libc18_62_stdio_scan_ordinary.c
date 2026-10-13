// (Test) Status: 0
// A test file for <stdio.h> ordinary characters and %% in a scanf format.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.2p6] An ordinary character should read its like, or fail.
static void Test_StdioScanOrdinary(void)
{
    int n = NONE;

    _TEST_TRUE(scans("a:b", "a:%s", 1, "b"));
    _TEST_TRUE(scans("a;b", "a:%s", 0, ""));
    _TEST_TRUE(scans("a", "a:%s", EOF, ""));
    _TEST_TRUE(scans("", "a%s", EOF, ""));
    _TEST_TRUE(scans(" a b", "a%s", 0, ""));
    _TEST_TRUE(scans("ab c", "a%s", 1, "b"));
    _TEST_INT(sscanf("abc", "ab%n", &n), 0);
    _TEST_INT(n, 2);
    n = NONE;
    _TEST_INT(sscanf("abd", "abc%n", &n), 0);
    _TEST_INT(n, NONE);

    _TEST_TRUE(scans("%x", "%%%s", 1, "x"));
    _TEST_TRUE(scans("  \n%x", "%%%s", 1, "x"));
    _TEST_TRUE(scans("x", "%%%s", 0, ""));
    _TEST_TRUE(scans("", "%%%s", EOF, ""));
    _TEST_TRUE(scans("%%x", "%%%%%s", 1, "x"));
    _TEST_INT(sscanf("100%", "100%%%n", &n), 0);
    _TEST_INT(n, 4);
}

int main(void)
{
    Test_StdioScanOrdinary();
    return _Test_Status();
}
