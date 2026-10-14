// (Test) Status: 0
// A test file for <stdio.h> what scanf returns when a directive fails.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.2p16] scanf should return EOF only if input ends before an item.
static void Test_StdioScanFailure(void)
{
    char ch = FILL;
    char first[SMALL];
    char second[SMALL];

    _TEST_INT(sscanf("", "%s", first), EOF);
    _TEST_INT(sscanf("", "%c", &ch), EOF);
    _TEST_INT(sscanf("", "%[a]", first), EOF);
    _TEST_INT(sscanf("", "a"), EOF);
    _TEST_INT(sscanf("", "%%"), EOF);
    _TEST_INT(sscanf("  \n", "%s", first), EOF);
    _TEST_INT(sscanf("  \n", " %c", &ch), EOF);
    _TEST_INT(sscanf("", " "), 0);
    _TEST_INT(sscanf("", ""), 0);

    _TEST_INT(sscanf("b", "a"), 0);
    _TEST_INT(sscanf("b", "%[a]", first), 0);
    _TEST_INT(sscanf("b", "%%"), 0);
    _TEST_INT(sscanf("b", "a%s", first), 0);

    _TEST_INT(sscanf("one", "%s%s", first, second), 1);
    _TEST_INT(sscanf("one", "%s%c", first, &ch), 1);
    _TEST_INT(sscanf("one ", "%s %[a]", first, second), 1);
    _TEST_INT(sscanf("one two", "%s%[a]", first, second), 1);
    _TEST_INT(sscanf("one two", "%s two%s", first, second), 1);
    _TEST_INT(sscanf("one two", "%s %s three", first, second), 2);
    _TEST_INT(sscanf("one two", "%s %s%c", first, second, &ch), 2);
    _TEST_INT(ch, FILL);
}

int main(void)
{
    Test_StdioScanFailure();
    return _Test_Status();
}
