// (Test) Status: 0
// A test file for <stdio.h> buffering modes.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

static char filebuf[SMALL];

// (S7.19.5.6p2) A stream buffered one way should pass lines past its size.
static bool buffers(int kind)
{
    int lost = 0;
    char *buf = kind == _IONBF ? NULL : filebuf;
    FILE *file = fopen("file", "w");

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! _TEST_TRUE(fputs(LONGER, file) >= 0);
    lost += ! _TEST_TRUE(fputs("two\n", file) >= 0);
    lost += ! _TEST_INT(fclose(file), 0);
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! reads(file, LONGER);
    lost += ! reads(file, "two\n");
    lost += ! _TEST_INT(fgetc(file), EOF);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// (S7.19.5.6p2) A stream should write and read the same however buffered.
static void Test_StdioBuffering(void)
{
    _TEST_TRUE(buffers(_IOFBF));
    _TEST_TRUE(buffers(_IOLBF));
    _TEST_TRUE(buffers(_IONBF));
}

int main(void)
{
    Test_StdioBuffering();
    return _Test_Status();
}
