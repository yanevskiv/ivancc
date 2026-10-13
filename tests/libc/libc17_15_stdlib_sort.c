// (Test) Status: 0
// A test file for <stdlib.h> qsort.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

#define KEYS 7

static bool seen[SORTED];
static unsigned char records[SORTED][3];

// Order three-byte records by their first byte alone.
static int compare_first(const void *left, const void *right)
{
    return *(const unsigned char *) left - *(const unsigned char *) right;
}

// A record should be whole, once and in order: return the first place not.
static int misplaced(void)
{
    int i;
    int index;

    for (i = 0; i < SORTED; i++) {
        index = records[i][1] | records[i][2] << CHAR_BIT;
        if (i > 0 && records[i - 1][0] > records[i][0]) {
            return i;
        }
        if (index >= SORTED || seen[index]) {
            return i;
        }
        seen[index] = true;
    }
    return NONE;
}

// [C99 S7.20.5.2p2] qsort should order by the comparison alone, each element whole.
static void Test_StdlibSort(void)
{
    int i;
    long sum = 0;
    long squares = 0;

    for (i = 0; i < SORTED; i++) {
        sorted[i] = (int) (next() % RANGE) - RANGE / 2;
        sum += sorted[i];
        squares += (long) sorted[i] * sorted[i];
    }
    qsort(sorted, SORTED, sizeof(sorted[0]), compare_int);
    _TEST_INT(descent(), NONE);
    for (i = 0; i < SORTED; i++) {
        sum -= sorted[i];
        squares -= (long) sorted[i] * sorted[i];
    }
    _TEST_INT(sum, 0);
    _TEST_INT(squares, 0);

    for (i = 0; i < SORTED; i++) {
        records[i][0] = (unsigned char) (next() % KEYS);
        records[i][1] = (unsigned char) (i & UCHAR_MAX);
        records[i][2] = (unsigned char) (i >> CHAR_BIT);
    }
    (qsort)(records, SORTED, sizeof(records[0]), compare_first);
    _TEST_INT(misplaced(), NONE);

    qsort(sorted, 0, sizeof(sorted[0]), compare_int);
    qsort(sorted, 1, sizeof(sorted[0]), compare_int);
    _TEST_INT(descent(), NONE);
}

int main(void)
{
    Test_StdlibSort();
    return _Test_Status();
}
