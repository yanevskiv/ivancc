// (Test) Status: 0
// A test file for <stdlib.h> calloc.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define ZEROED 256

// [C99/S7.20.3.1p2] calloc should give zeroed memory, of a block freed too.
static void Test_StdlibCalloc(void)
{
    unsigned char *ptr = malloc(ZEROED);
    static const unsigned char zeros[ZEROED];

    if (_TEST_TRUE(ptr != NULL)) {
        memset(ptr, UCHAR_MAX, ZEROED);
        free(ptr);
    }
    ptr = calloc(SMALL, ZEROED / SMALL);
    if (_TEST_TRUE(ptr != NULL)) {
        _TEST_UINT(misaligned(ptr), 0);
        _TEST_MEM(ptr, zeros, ZEROED);
        free(ptr);
    }
    free(calloc(0, SMALL));
    _TEST_TRUE(calloc(huge / 2 + 2, 2) == NULL);
}

int main(void)
{
    Test_StdlibCalloc();
    return _Test_Status();
}
