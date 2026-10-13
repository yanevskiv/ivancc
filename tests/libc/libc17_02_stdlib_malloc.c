// (Test) Status: 0
// A test file for <stdlib.h> malloc.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define SIZES 10
#define BIG   (1 << 17)

// The sizes of the blocks that live together, either side of a power of two.
static const size_t sizes[SIZES] = {1, 15, 16, 17, 31, 32, 33, 100, 1000, 4096};

// [C99:S7.20.3p1] Each size should get an aligned block: return the first without.
static long allocate(void)
{
    size_t i;

    for (i = 0; i < SIZES; i++) {
        slot[i] = malloc(sizes[i]);
        if (slot[i] == NULL || misaligned(slot[i]) != 0) {
            return (long) sizes[i];
        }
        fill(slot[i], sizes[i], (unsigned char) i);
    }
    return NONE;
}

// A block should keep its bytes till freed: return the first size that did not.
static long release(void)
{
    size_t i;
    long changed = NONE;

    for (i = 0; i < SIZES; i++) {
        if (changed == NONE && spoiled(slot[i], sizes[i], (unsigned char) i) != NONE) {
            changed = (long) sizes[i];
        }
        free(slot[i]);
        slot[i] = NULL;
    }
    return changed;
}

// [C99:S7.20.3.3p2] malloc should give aligned blocks that keep their bytes.
static void Test_StdlibMalloc(void)
{
    void *empty;
    void *other;
    unsigned char *big;

    if (_TEST_INT(allocate(), NONE)) {
        _TEST_INT(release(), NONE);
    }

    empty = malloc(0);
    other = malloc(0);
    _TEST_TRUE(empty == NULL || empty != other);
    free(empty);
    free(other);
    free(NULL);

    big = malloc(BIG);
    if (_TEST_TRUE(big != NULL)) {
        _TEST_UINT(misaligned(big), 0);
        fill(big, BIG, 6);
        _TEST_INT(spoiled(big, BIG, 6), NONE);
        free(big);
    }
    _TEST_TRUE(malloc(huge) == NULL);
}

int main(void)
{
    Test_StdlibMalloc();
    return _Test_Status();
}
