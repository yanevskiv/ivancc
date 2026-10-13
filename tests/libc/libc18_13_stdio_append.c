// (Test) Status: 0
// A test file for <stdio.h> append streams.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.5.3p5] An append stream should write at its file's end, wherever set.
static void Test_StdioAppend(void)
{
    FILE *file;

    _TEST_TRUE(put("pos", "w", "abc"));
    file = fopen("pos", "a");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fseek(file, 0, SEEK_SET), 0);
    _TEST_TRUE(fputs("def", file) >= 0);
    _TEST_INT(fclose(file), 0);
    file = fopen("pos", "a+b");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fseek(file, 1, SEEK_SET), 0);
    _TEST_INT(fgetc(file), 'b');
    _TEST_INT(fseek(file, 0, SEEK_SET), 0);
    _TEST_INT(fputc('g', file), 'g');
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("pos", "abcdefg"));
}

int main(void)
{
    Test_StdioAppend();
    return _Test_Status();
}
