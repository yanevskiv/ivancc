// (Test) Status: 0
// A test file for <stdlib.h> bsearch.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define STRIDE 7

// (S7.20.5.1p2) bsearch should find each int: return the first place it fails.
static int unfound(void)
{
    int i;
    const int *found;

    for (i = 0; i < SORTED; i += STRIDE) {
        found = bsearch(&sorted[i], sorted, SORTED, sizeof(sorted[0]), compare_int);
        if (found == NULL || *found != sorted[i]) {
            return i;
        }
    }
    return NONE;
}

// (S7.20.5.1p2) bsearch should find an element of the sorted ints, no other.
static void Test_StdlibSearch(void)
{
    int i;
    int missing = RANGE;

    for (i = 0; i < SORTED; i++) {
        sorted[i] = (int) (next() % RANGE) - RANGE / 2;
    }
    qsort(sorted, SORTED, sizeof(sorted[0]), compare_int);
    if (! _TEST_INT(descent(), NONE)) {
        return;
    }
    _TEST_INT(unfound(), NONE);
    _TEST_TRUE(bsearch(&missing, sorted, SORTED, sizeof(sorted[0]), compare_int) == NULL);
    _TEST_TRUE((bsearch)(&sorted[0], sorted, 0, sizeof(sorted[0]), compare_int) == NULL);
}

int main(void)
{
    Test_StdlibSearch();
    return _Test_Status();
}
