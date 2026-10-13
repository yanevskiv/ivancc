// (Test) Status: 0
// A test file for <stdio.h> the n conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.2p12] %n should store the characters read so far and count nothing.
static void Test_StdioScanCount(void)
{
    int n = NONE;
    int m = NONE;
    char word[SMALL];

    _TEST_INT(sscanf("abc", "%n", &n), 0);
    _TEST_INT(n, 0);
    _TEST_INT(sscanf("", "%n", &n), 0);
    _TEST_INT(n, 0);
    _TEST_INT(sscanf("ab", "%n%s%n", &n, word, &m), 1);
    _TEST_INT(n, 0);
    _TEST_INT(m, 2);
    _TEST_STR(word, "ab");
    _TEST_INT(sscanf("  ab  cd", "%s%n%s%n", word, &n, word, &m), 2);
    _TEST_INT(n, 4);
    _TEST_INT(m, 8);
    _TEST_INT(sscanf("abcde", "%*2c%n%*[c]%n", &n, &m), 0);
    _TEST_INT(n, 2);
    _TEST_INT(m, 3);
    n = NONE;
    _TEST_INT(sscanf("x", "%[a]%n", word, &n), 0);
    _TEST_INT(n, NONE);
}

int main(void)
{
    Test_StdioScanCount();
    return _Test_Status();
}
