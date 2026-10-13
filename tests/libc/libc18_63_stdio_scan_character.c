// (Test) Status: 0
// A test file for <stdio.h> the c conversion of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.2p12) %c should store a width's characters and no null after them.
static bool takes(const char *input, const char *format, const char *want)
{
    int lost = 0;
    char got[SMALL];
    size_t len = strlen(want);

    memset(got, FILL, sizeof(got));
    lost += ! _TEST_INT(sscanf(input, format, got), 1);
    lost += ! _TEST_MEM(got, want, len);
    lost += ! _TEST_INT(got[len], FILL);
    return lost == 0;
}

// (S7.19.6.2p12) %c should match exactly its width of characters, 1 if none.
static void Test_StdioScanCharacter(void)
{
    char a = FILL;
    char b = FILL;

    _TEST_TRUE(takes("abc", "%c", "a"));
    _TEST_TRUE(takes("abc", "%1c", "a"));
    _TEST_TRUE(takes("abc", "%2c", "ab"));
    _TEST_TRUE(takes("abc", "%3c", "abc"));
    _TEST_TRUE(takes(" a", "%c", " "));
    _TEST_TRUE(takes("\n", "%c", "\n"));
    _TEST_TRUE(takes("a b\tc", "%5c", "a b\tc"));
    _TEST_TRUE(takes("abcdefghijkl", "%10c", "abcdefghij"));
    _TEST_INT(sscanf("", "%c", &a), EOF);
    _TEST_INT(sscanf("xy", "%c%c", &a, &b), 2);
    _TEST_INT(a, 'x');
    _TEST_INT(b, 'y');
    _TEST_INT(sscanf("p", "%c%c", &a, &b), 1);
    _TEST_INT(a, 'p');
    _TEST_INT(b, 'y');
}

int main(void)
{
    Test_StdioScanCharacter();
    return _Test_Status();
}
