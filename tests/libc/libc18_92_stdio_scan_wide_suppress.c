// (Test) Status: 0
// A test file for <stdio.h> scanf's wide conversions that store nothing.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.2p10] An asterisk should read a wide item and store nothing.
// [C99 S7.19.6.2p16] Input ending before any conversion should return EOF.
static void Test_StdioScanWideSuppress(void)
{
    int n = NONE;
    wchar_t wc = FILL;
    wchar_t word[SMALL];

    _TEST_INT(sscanf("abc", "%*lc%lc", &wc), 1);
    _TEST_INT(wc, L'b');
    _TEST_INT(sscanf("abc", "%*2lc%lc%n", &wc, &n), 1);
    _TEST_INT(wc, L'c');
    _TEST_INT(n, 3);
    _TEST_INT(sscanf("one two", "%*ls%ls%n", word, &n), 1);
    _TEST_MEM(word, L"two", 4 * sizeof(wchar_t));
    _TEST_INT(n, 7);
    _TEST_INT(sscanf("aab", "%*l[a]%lc", &wc), 1);
    _TEST_INT(wc, L'b');
    _TEST_INT(sscanf("abc de f", "%*lc%*ls%*l[ de]%n", &n), 0);
    _TEST_INT(n, 7);
    n = NONE;
    _TEST_INT(sscanf("", "%*lc%n", &n), EOF);
    _TEST_INT(sscanf(" ", "%*ls%n", &n), EOF);
    _TEST_INT(sscanf("", "%*l[a]%n", &n), EOF);
    _TEST_INT(sscanf("b", "%*l[a]%n", &n), 0);
    _TEST_INT(n, NONE);
}

int main(void)
{
    Test_StdioScanWideSuppress();
    return _Test_Status();
}
