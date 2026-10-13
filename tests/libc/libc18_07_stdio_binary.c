// (Test) Status: 0
// A test file for <stdio.h> binary streams.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define BIG (3 * BUFSIZ + 5)

// [C99 S7.19.7.3p3] fputc should write BIG bytes: return the first it fails.
static long unwritten(FILE *file)
{
    long k;

    for (k = 0; k < BIG; k++) {
        if (fputc((int) (k % PRIME), file) == EOF) {
            return k;
        }
    }
    return NONE;
}

// [C99 S7.19.7.1p2] fgetc should read BIG bytes back: return the first to differ.
static long misread(FILE *file)
{
    long k;

    for (k = 0; k < BIG; k++) {
        if (fgetc(file) != (int) (k % PRIME)) {
            return k;
        }
    }
    return NONE;
}

// The wide block should hold its pattern: return the first byte that does not.
static long unpatterned(void)
{
    size_t k;

    for (k = 0; k < sizeof(wide); k++) {
        if (wide[k] != k % PRIME) {
            return (long) k;
        }
    }
    return NONE;
}

// [C99 S7.19.2p3] A binary stream should keep every byte, past its buffer's size.
static void Test_StdioBinary(void)
{
    FILE *file = fopen("bytes", "wb");
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_UINT(fwrite(bytes, 1, sizeof(bytes), file), sizeof(bytes));
    _TEST_INT(fclose(file), 0);
    file = fopen("bytes", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_UINT(fread(block, 1, sizeof(block), file), sizeof(bytes));
    _TEST_MEM(block, bytes, sizeof(bytes));
    _TEST_TRUE(feof(file));
    _TEST_INT(fclose(file), 0);

    file = fopen("big", "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(unwritten(file), NONE);
    _TEST_INT(fclose(file), 0);
    file = fopen("big", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(misread(file), NONE);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);

    pattern();
    file = fopen("big", "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fputc('!', file), '!');
    _TEST_UINT(fwrite(wide, sizeof(wide), 1, file), 1);
    _TEST_INT(fclose(file), 0);
    memset(wide, 0, sizeof(wide));
    file = fopen("big", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fgetc(file), '!');
    _TEST_UINT(fread(wide, 1, sizeof(wide), file), sizeof(wide));
    _TEST_INT(unpatterned(), NONE);
    _TEST_INT(fgetc(file), EOF);
    _TEST_INT(fclose(file), 0);
}

int main(void)
{
    Test_StdioBinary();
    return _Test_Status();
}
