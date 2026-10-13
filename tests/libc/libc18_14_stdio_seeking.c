// (Test) Status: 0
// A test file for <stdio.h> fseek within a buffer and past it.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.9.2p2] fseek should move within a stream's buffer and past it.
static void Test_StdioSeeking(void)
{
    FILE *file;

    pattern();
    file = fopen("big", "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fputc('!', file), '!');
    _TEST_UINT(fwrite(wide, sizeof(wide), 1, file), 1);
    _TEST_INT(fclose(file), 0);
    file = fopen("big", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fseek(file, BUFSIZ + 50, SEEK_SET), 0);
    _TEST_INT(fgetc(file), (BUFSIZ + 49) % PRIME);
    _TEST_INT(ftell(file), BUFSIZ + 51);
    _TEST_INT(fseek(file, -(long) BUFSIZ, SEEK_CUR), 0);
    _TEST_INT(fgetc(file), 50);
    _TEST_INT(fseek(file, 3, SEEK_CUR), 0);
    _TEST_INT(fgetc(file), 54);
    _TEST_INT(ftell(file), 56);
    _TEST_INT(fclose(file), 0);
}

int main(void)
{
    Test_StdioSeeking();
    return _Test_Status();
}
