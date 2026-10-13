// (Test) Status: 0
// A test file for <stdio.h> update streams.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define MODES 15

// fopen's modes, the last of which leaves its file empty.
static const char *const modes[MODES] = {"r", "rb", "r+", "rb+", "r+b", "a", "ab", "a+", "ab+", "a+b", "w", "wb", "w+", "wb+", "w+b"};

// [C99:S7.19.5.3p3] A file should open in each mode: return the first it does not.
static int unopened(void)
{
    int k;
    FILE *file;

    for (k = 0; k < MODES; k++) {
        file = fopen("file", modes[k]);
        if (file == NULL || fclose(file) != 0) {
            return k;
        }
    }
    return NONE;
}

// [C99:S7.19.5.3p6] Update streams should write after a flush or a read to the end.
static void Test_StdioUpdate(void)
{
    FILE *file;
    char block[HOLD];

    if (! _TEST_TRUE(put("file", "w", "abcdef"))) {
        return;
    }
    file = fopen("file", "r+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs("XY", file) >= 0);
    _TEST_INT(fflush(file), 0);
    _TEST_INT(fgetc(file), 'c');
    _TEST_UINT(fread(block, 1, 8, file), 3);
    _TEST_MEM(block, "def", 3);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(fputs("gh", file) >= 0);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("file", "XYcdefgh"));

    file = fopen("file", "w+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs("hello", file) >= 0);
    _TEST_INT(fflush(file), 0);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    _TEST_INT(fclose(file), 0);

    file = fopen("file", "a+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs(", world", file) >= 0);
    _TEST_INT(fflush(file), 0);
    _TEST_INT(fgetc(file), EOF);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("file", "hello, world"));

    _TEST_INT(unopened(), NONE);
    _TEST_TRUE(holds("file", ""));
}

int main(void)
{
    Test_StdioUpdate();
    return _Test_Status();
}
