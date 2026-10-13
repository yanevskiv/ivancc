// (Test) Status: 0
// A test file for <stdlib.h> realloc.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define BLOCK  100
#define GROWN  5000
#define SHRUNK 10
#define WIDER  180

// [C99 S7.20.3.4p2] realloc should keep a block's bytes as it grows or shrinks.
static void Test_StdlibRealloc(void)
{
    unsigned char *ptr = realloc(NULL, BLOCK);
    unsigned char *other;

    if (! _TEST_TRUE(ptr != NULL)) {
        return;
    }
    _TEST_UINT(misaligned(ptr), 0);
    fill(ptr, BLOCK, 3);
    _TEST_TRUE(realloc(ptr, huge) == NULL);
    _TEST_INT(spoiled(ptr, BLOCK, 3), NONE);
    ptr = realloc(ptr, GROWN);
    if (! _TEST_TRUE(ptr != NULL)) {
        return;
    }
    _TEST_UINT(misaligned(ptr), 0);
    _TEST_INT(spoiled(ptr, BLOCK, 3), NONE);
    fill(ptr, GROWN, 4);
    ptr = realloc(ptr, SHRUNK);
    if (! _TEST_TRUE(ptr != NULL)) {
        return;
    }
    _TEST_INT(spoiled(ptr, SHRUNK, 4), NONE);
    free(ptr);

    ptr = malloc(BLOCK);
    other = malloc(BLOCK);
    if (! _TEST_TRUE(ptr != NULL && other != NULL)) {
        return;
    }
    fill(ptr, BLOCK, 5);
    free(other);
    ptr = realloc(ptr, WIDER);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_INT(spoiled(ptr, BLOCK, 5), NONE);
        free(ptr);
    }
}

int main(void)
{
    Test_StdlibRealloc();
    return _Test_Status();
}
