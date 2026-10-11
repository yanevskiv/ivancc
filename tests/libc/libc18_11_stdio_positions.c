// (Test) Status: 0
// A test file for <stdio.h> positions in a binary stream.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// (S7.19.9.4p2) A binary stream's position should count characters.
static void Test_StdioPositions(void)
{
    fpos_t pos;
    FILE *file = fopen("pos", "w+b");
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(ftell(file), 0);
    _TEST_UINT(fwrite(DIGITS, 1, 36, file), 36);
    _TEST_INT(ftell(file), 36);
    rewind(file);
    _TEST_INT(ftell(file), 0);
    _TEST_INT(fgetc(file), '0');
    _TEST_INT(ftell(file), 1);
    _TEST_INT(fseek(file, 10, SEEK_SET), 0);
    _TEST_INT(fgetc(file), 'a');
    _TEST_INT(ftell(file), 11);
    _TEST_INT(fseek(file, 5, SEEK_CUR), 0);
    _TEST_INT(ftell(file), 16);
    _TEST_INT(fgetc(file), 'g');
    _TEST_INT(fseek(file, -7, SEEK_CUR), 0);
    _TEST_INT(ftell(file), 10);
    _TEST_INT(fgetc(file), 'a');

    _TEST_INT(ungetc('?', file), '?');
    _TEST_INT(ftell(file), 10);
    _TEST_INT(fseek(file, 0, SEEK_CUR), 0);
    _TEST_INT(fgetc(file), 'a');
    _TEST_UINT(fread(block, 1, sizeof(block), file), 25);
    _TEST_TRUE(feof(file));
    _TEST_INT(fseek(file, 0, SEEK_CUR), 0);
    _TEST_TRUE(! feof(file));
    _TEST_INT(ftell(file), 36);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    rewind(file);
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fgetc(file), '0');
    _TEST_TRUE(fseek(file, -1, SEEK_SET) != 0);

    _TEST_INT(fseek(file, 10, SEEK_SET), 0);
    _TEST_UINT(fread(block, 1, 2, file), 2);
    _TEST_MEM(block, "ab", 2);
    _TEST_INT(fseek(file, 0, SEEK_CUR), 0);
    _TEST_UINT(fwrite("XY", 1, 2, file), 2);
    _TEST_INT(ftell(file), 14);
    _TEST_INT(fseek(file, -4, SEEK_CUR), 0);
    _TEST_UINT(fread(block, 1, 6, file), 6);
    _TEST_MEM(block, "abXYef", 6);
    _TEST_INT(fgetpos(file, &pos), 0);
    _TEST_INT(fgetc(file), 'g');
    _TEST_INT(fgetc(file), 'h');
    _TEST_INT(fsetpos(file, &pos), 0);
    _TEST_INT(fgetc(file), 'g');
    _TEST_INT(ftell(file), 17);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("pos", "0123456789abXYefghijklmnopqrstuvwxyz"));
}

int main(void)
{
    Test_StdioPositions();
    return _Test_Status();
}
