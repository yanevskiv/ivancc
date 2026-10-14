// (Test) Status: 0
// A test file for <stdio.h> the l[ conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.2p12] %l[ should read a scanset's characters into wide ones and a
// null, and fail if the first character is not of the scanset.
static void Test_StdioScanWideSet(void)
{
    int n = NONE;
    wchar_t first[SMALL];
    wchar_t second[SMALL];

    _TEST_TRUE(scans_wide("abcd", "%l[abc]", 1, L"abc", 4));
    _TEST_TRUE(scans_wide("hello world", "%l[^ ]", 1, L"hello", 6));
    _TEST_TRUE(scans_wide("hello", "%l[^l]", 1, L"he", 3));
    _TEST_TRUE(scans_wide(" a b", "%l[ a]", 1, L" a ", 4));
    _TEST_TRUE(scans_wide("]ab", "%l[]a]", 1, L"]a", 3));
    _TEST_TRUE(scans_wide("ab]", "%l[^]]", 1, L"ab", 3));
    _TEST_TRUE(scans_wide("a^b", "%l[a^]", 1, L"a^", 3));
    _TEST_TRUE(scans_wide("aaaa", "%2l[a]", 1, L"aa", 3));
    _TEST_TRUE(scans_wide("abc", "%9l[abc]", 1, L"abc", 4));
    _TEST_TRUE(scans_wide("x", "%l[abc]", 0, L"", 0));
    _TEST_TRUE(scans_wide(" a", "%l[a]", 0, L"", 0));
    _TEST_TRUE(scans_wide("a", "%l[^a]", 0, L"", 0));
    _TEST_TRUE(scans_wide("", "%l[abc]", EOF, L"", 0));
    _TEST_TRUE(scans_wide("", "%l[^a]", EOF, L"", 0));
    _TEST_INT(sscanf("key=val", "%l[^=]=%l[^=]%n", first, second, &n), 2);
    _TEST_MEM(first, L"key", 4 * sizeof(wchar_t));
    _TEST_MEM(second, L"val", 4 * sizeof(wchar_t));
    _TEST_INT(n, 7);
    _TEST_INT(sscanf("12ab", "%l[12]%l[12]", first, second), 1);
    _TEST_MEM(first, L"12", 3 * sizeof(wchar_t));
}

int main(void)
{
    Test_StdioScanWideSet();
    return _Test_Status();
}
