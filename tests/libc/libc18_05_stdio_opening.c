// (Test) Status: 0
// A test file for <stdio.h> fopen.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.5.3p3] fopen should make, extend and empty a file, and read none lost.
static void Test_StdioOpening(void)
{
    FILE *file;

    _TEST_TRUE(fopen("missing", "r") == NULL);
    _TEST_TRUE(fopen("missing", "rb") == NULL);
    _TEST_TRUE(fopen("missing", "r+") == NULL);
    file = fopen("file", "w");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(file != stdin && file != stdout && file != stderr);
    _TEST_TRUE(fputs("abcdef\n", file) >= 0);
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("file", "abcdef\n"));
    _TEST_TRUE(put("file", "a", "gh\n"));
    _TEST_TRUE(holds("file", "abcdef\ngh\n"));
    _TEST_TRUE(put("file", "w", "abcdef"));
    _TEST_TRUE(holds("file", "abcdef"));
    _TEST_TRUE(put("made by a", "a", "a"));
    _TEST_TRUE(put("made by w+", "w+", "w+"));
    _TEST_TRUE(put("made by a+", "a+", "a+"));
    _TEST_TRUE(holds("made by a", "a"));
    _TEST_TRUE(holds("made by w+", "w+"));
    _TEST_TRUE(holds("made by a+", "a+"));
}

int main(void)
{
    Test_StdioOpening();
    return _Test_Status();
}
