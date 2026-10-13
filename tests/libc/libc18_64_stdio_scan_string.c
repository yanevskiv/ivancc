// (Test) Status: 0
// A test file for <stdio.h> the s conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.2p12] %s should store a run of no white space and a null after it.
static void Test_StdioScanString(void)
{
    char first[SMALL];
    char second[SMALL];

    _TEST_TRUE(scans("word", "%s", 1, "word"));
    _TEST_TRUE(scans("two words", "%s", 1, "two"));
    _TEST_TRUE(scans("tab\tbed", "%s", 1, "tab"));
    _TEST_TRUE(scans("line\nfed", "%s", 1, "line"));
    _TEST_TRUE(scans("%[^]", "%s", 1, "%[^]"));
    _TEST_TRUE(scans("abcdef", "%3s", 1, "abc"));
    _TEST_TRUE(scans("abcdef", "%1s", 1, "a"));
    _TEST_TRUE(scans("abcdef", "%6s", 1, "abcdef"));
    _TEST_TRUE(scans("ab cd", "%5s", 1, "ab"));
    _TEST_TRUE(scans("   abcdef", "%3s", 1, "abc"));
    _TEST_TRUE(scans("abcdef", "%020s", 1, "abcdef"));
    _TEST_TRUE(scans("", "%s", EOF, ""));
    _TEST_TRUE(scans(" \n", "%s", EOF, ""));
    _TEST_INT(sscanf("abcdef", "%3s%s", first, second), 2);
    _TEST_STR(first, "abc");
    _TEST_STR(second, "def");
    _TEST_INT(sscanf("ab", "%2s%s", first, second), 1);
    _TEST_STR(first, "ab");
}

int main(void)
{
    Test_StdioScanString();
    return _Test_Status();
}
