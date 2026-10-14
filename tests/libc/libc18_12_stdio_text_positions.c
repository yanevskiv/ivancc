// (Test) Status: 0
// A test file for <stdio.h> positions in a text stream.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.9.2p4] A text stream should return to where ftell said it was.
static void Test_StdioTextPositions(void)
{
    long at;
    FILE *file;

    _TEST_TRUE(put("pos", "w", "one\ntwo\nthree\n"));
    file = fopen("pos", "r+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(reads(file, "one\n"));
    at = ftell(file);
    _TEST_TRUE(at >= 0);
    _TEST_TRUE(reads(file, "two\n"));
    _TEST_TRUE(reads(file, "three\n"));
    _TEST_INT(fseek(file, at, SEEK_SET), 0);
    _TEST_TRUE(reads(file, "two\n"));
    _TEST_INT(fseek(file, 0, SEEK_END), 0);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    _TEST_INT(fseek(file, 0, SEEK_END), 0);
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(fputs("four\n", file) >= 0);
    _TEST_INT(fseek(file, at, SEEK_SET), 0);
    _TEST_TRUE(reads(file, "two\n"));
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("pos", "one\ntwo\nthree\nfour\n"));
}

int main(void)
{
    Test_StdioTextPositions();
    return _Test_Status();
}
