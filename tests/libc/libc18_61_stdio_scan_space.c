// (Test) Status: 0
// A test file for <stdio.h> white space in a scanf format and its input.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.6.2p5) White space in a format should read input's, however much.
static void Test_StdioScanSpace(void)
{
    int n = NONE;
    char ch = FILL;

    _TEST_TRUE(scans("   word", " %s", 1, "word"));
    _TEST_TRUE(scans("word", " %s", 1, "word"));
    _TEST_TRUE(scans("\t\n\v\f\r word", " %s", 1, "word"));
    _TEST_TRUE(scans("a   word", "a \t\n%s", 1, "word"));
    _TEST_INT(sscanf("a   b", "a %n", &n), 0);
    _TEST_INT(n, 4);
    _TEST_INT(sscanf("a   ", "a %n", &n), 0);
    _TEST_INT(n, 4);
    _TEST_INT(sscanf("a", "a %n", &n), 0);
    _TEST_INT(n, 1);
    _TEST_INT(sscanf("", " %n", &n), 0);
    _TEST_INT(n, 0);

    _TEST_TRUE(scans("  \n word", "%s", 1, "word"));
    _TEST_INT(sscanf("word  ", "%*s%n", &n), 0);
    _TEST_INT(n, 4);
    _TEST_INT(sscanf(" x", "%c", &ch), 1);
    _TEST_INT(ch, ' ');
    _TEST_INT(sscanf(" x", " %c", &ch), 1);
    _TEST_INT(ch, 'x');
    _TEST_TRUE(scans(" a", "%[a]", 0, ""));
    _TEST_TRUE(scans(" a", " %[a]", 1, "a"));
    _TEST_INT(sscanf(" x", "%n%c", &n, &ch), 1);
    _TEST_INT(n, 0);
    _TEST_INT(ch, ' ');
}

int main(void)
{
    Test_StdioScanSpace();
    return _Test_Status();
}
