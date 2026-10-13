// (Test) Status: 0
// A test file for <stdio.h> tmpfile.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.4.3p2] Each of tmpfile's streams should update a file of its own.
static void Test_StdioTmpfile(void)
{
    FILE *file = tmpfile();
    FILE *other = tmpfile();
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL && other != NULL)) {
        return;
    }
    _TEST_TRUE(file != other);
    _TEST_UINT(fwrite(bytes, 1, sizeof(bytes), file), sizeof(bytes));
    _TEST_TRUE(fputs("the other's", other) >= 0);
    rewind(file);
    rewind(other);
    _TEST_UINT(fread(block, 1, sizeof(block), file), sizeof(bytes));
    _TEST_MEM(block, bytes, sizeof(bytes));
    _TEST_TRUE(reads(other, "the other's"));
    _TEST_INT(fseek(file, 2, SEEK_SET), 0);
    _TEST_INT(fputc('!', file), '!');
    _TEST_INT(fseek(file, 2, SEEK_SET), 0);
    _TEST_INT(fgetc(file), '!');
    _TEST_INT(fclose(file), 0);
    _TEST_INT(fclose(other), 0);
}

int main(void)
{
    Test_StdioTmpfile();
    return _Test_Status();
}
