// (Test) Status: 0
// A test file for <stdio.h> the assignment-suppressing character of scanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.2p10] An asterisk should convert, assign nothing and count nothing.
static void Test_StdioScanSuppress(void)
{
    int n = NONE;
    char ch = FILL;

    _TEST_TRUE(scans("one two", "%*s%s", 1, "two"));
    _TEST_TRUE(scans("aab", "%*[a]%s", 1, "b"));
    _TEST_TRUE(scans("abcdef", "%*3c%s", 1, "def"));
    _TEST_TRUE(scans("abcdef", "%*2s%*2[c-]%s", 1, "def"));
    _TEST_TRUE(scans("b", "%*[a]%s", 0, ""));
    _TEST_INT(sscanf("abc", "%*c%c", &ch), 1);
    _TEST_INT(ch, 'b');
    _TEST_INT(sscanf("one", "%*s"), 0);
    _TEST_INT(sscanf("one two", "%*s%n", &n), 0);
    _TEST_INT(n, 3);
    _TEST_INT(sscanf("one two", "%*2c%n", &n), 0);
    _TEST_INT(n, 2);
    _TEST_INT(sscanf("one two", "%*s %*[tw]%n", &n), 0);
    _TEST_INT(n, 6);
    _TEST_INT(sscanf("", "%*s"), EOF);
}

int main(void)
{
    Test_StdioScanSuppress();
    return _Test_Status();
}
