// (Test) Status: 0
// A test file for <stdio.h> the ls conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.2p12] %ls should read a word into wide characters and a null.
// [C99:7.19.6.2p8] %ls should skip the white space before the word.
static void Test_StdioScanWideString(void)
{
    int n = NONE;
    int value = NONE;
    wchar_t first[SMALL];
    wchar_t second[SMALL];

    _TEST_TRUE(scans_wide("hello", "%ls", 1, L"hello", 6));
    _TEST_TRUE(scans_wide("hello world", "%ls", 1, L"hello", 6));
    _TEST_TRUE(scans_wide("  \n\thello\n", "%ls", 1, L"hello", 6));
    _TEST_TRUE(scans_wide("a", "%ls", 1, L"a", 2));
    _TEST_TRUE(scans_wide("12,x%", "%ls", 1, L"12,x%", 6));
    _TEST_TRUE(scans_wide("hello", "%3ls", 1, L"hel", 4));
    _TEST_TRUE(scans_wide("hello", "%1ls", 1, L"h", 2));
    _TEST_TRUE(scans_wide("hi there", "%9ls", 1, L"hi", 3));
    _TEST_TRUE(scans_wide("   hello", "%2ls", 1, L"he", 3));
    _TEST_TRUE(scans_wide("", "%ls", EOF, L"", 0));
    _TEST_TRUE(scans_wide(" \n ", "%ls", EOF, L"", 0));
    _TEST_INT(sscanf("one two", "%ls%ls", first, second), 2);
    _TEST_MEM(first, L"one", 4 * sizeof(wchar_t));
    _TEST_MEM(second, L"two", 4 * sizeof(wchar_t));
    _TEST_INT(sscanf("abcdef", "%2ls%3ls%n", first, second, &n), 2);
    _TEST_MEM(first, L"ab", 3 * sizeof(wchar_t));
    _TEST_MEM(second, L"cde", 4 * sizeof(wchar_t));
    _TEST_INT(n, 5);
    _TEST_INT(sscanf("age 42", "%ls%d", first, &value), 2);
    _TEST_MEM(first, L"age", 4 * sizeof(wchar_t));
    _TEST_INT(value, 42);
    _TEST_INT(sscanf("solo ", "%ls%ls", first, second), 1);
    _TEST_MEM(first, L"solo", 5 * sizeof(wchar_t));
}

int main(void)
{
    Test_StdioScanWideString();
    return _Test_Status();
}
