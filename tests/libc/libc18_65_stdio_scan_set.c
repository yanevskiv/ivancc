// (Test) Status: 0
// A test file for <stdio.h> the [ conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// %[ should store a run of its scanset and a null after it.
// [C99:7.19.6.2p12]: scanf's conversion specifiers
static void Test_StdioScanSet(void)
{
    char first[SMALL];
    char second[SMALL];

    _TEST_TRUE(scans("abcd", "%[abc]", 1, "abc"));
    _TEST_TRUE(scans("cabd", "%[abc]", 1, "cab"));
    _TEST_TRUE(scans("abcd", "%[^d]", 1, "abc"));
    _TEST_TRUE(scans("a b\nc", "%[^\n]", 1, "a b"));
    _TEST_TRUE(scans("a b\nc", "%[ ab]", 1, "a b"));
    _TEST_TRUE(scans("a]b[", "%[]a]", 1, "a]"));
    _TEST_TRUE(scans("xy]z", "%[^]]", 1, "xy"));
    _TEST_TRUE(scans("xy]z", "%[^]y]", 1, "x"));
    _TEST_TRUE(scans("a-b", "%[-a]", 1, "a-"));
    _TEST_TRUE(scans("a-b", "%[a-]", 1, "a-"));
    _TEST_TRUE(scans("ab-c", "%[^-]", 1, "ab"));
    _TEST_TRUE(scans("a^b", "%[a^]", 1, "a^"));
    _TEST_TRUE(scans("[%]", "%[[%]", 1, "[%"));
    _TEST_TRUE(scans("aaaa", "%2[a]", 1, "aa"));
    _TEST_TRUE(scans("aab", "%5[a]", 1, "aa"));
    _TEST_TRUE(scans("x", "%[a]", 0, ""));
    _TEST_TRUE(scans("a", "%[^a]", 0, ""));
    _TEST_TRUE(scans(" a", "%[a]", 0, ""));
    _TEST_TRUE(scans("", "%[a]", EOF, ""));
    _TEST_TRUE(scans("", "%[^a]", EOF, ""));
    _TEST_INT(sscanf("key=some value", "%[^=]=%[^\n]", first, second), 2);
    _TEST_STR(first, "key");
    _TEST_STR(second, "some value");
    _TEST_INT(sscanf("aabb", "%[a]%[b]", first, second), 2);
    _TEST_STR(first, "aa");
    _TEST_STR(second, "bb");
}

int main(void)
{
    Test_StdioScanSet();
    return _Test_Status();
}
