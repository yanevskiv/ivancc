// (Test) Status: 0
// A test file for <stddef.h> offsetof macro.

// Include helpers.
#include "libc03_00_stddef_helpers.inc"

union Either {
    char ei_ch;
    long ei_number;
};

// Return how far a member lies into its structure, in bytes.
static long distance(const void *whole, const void *member)
{
    return (const char *) member - (const char *) whole;
}

// [C99 S7.17p3] offsetof should be a member's offset in bytes, a size_t constant.
static void Test_StddefOffsetof(void)
{
    char sized[offsetof(struct Pair, pa_tail)];
    struct Pair pair;

    _TEST_UINT(offsetof(struct Pair, pa_ch), 0);
    _TEST_UINT(offsetof(struct Pair, pa_number), 4);
    _TEST_UINT(offsetof(struct Pair, pa_tail), 8);
    _TEST_UINT(offsetof(struct Pair, pa_tail[1]), 9);
    _TEST_UINT(offsetof(struct Pair, pa_inner), 16);
    _TEST_UINT(offsetof(struct Pair, pa_inner.in_second), 24);
    _TEST_UINT(offsetof(struct Inner, in_second), 8);
    _TEST_UINT(offsetof(union Either, ei_ch), 0);
    _TEST_UINT(offsetof(union Either, ei_number), 0);

    _TEST_UINT(sizeof(offsetof(struct Pair, pa_number)), sizeof(size_t));
    _TEST_TRUE(offsetof(struct Pair, pa_ch) - 1 > 0);
    _TEST_UINT(sizeof(sized), 8);

    _TEST_INT(distance(&pair, &pair.pa_ch), offsetof(struct Pair, pa_ch));
    _TEST_INT(distance(&pair, &pair.pa_number), offsetof(struct Pair, pa_number));
    _TEST_INT(distance(&pair, &pair.pa_tail), offsetof(struct Pair, pa_tail));
    _TEST_INT(distance(&pair, &pair.pa_tail[1]), offsetof(struct Pair, pa_tail[1]));
    _TEST_INT(distance(&pair, &pair.pa_inner), offsetof(struct Pair, pa_inner));
    _TEST_INT(distance(&pair, &pair.pa_inner.in_second), offsetof(struct Pair, pa_inner.in_second));
}

int main(void)
{
    Test_StddefOffsetof();
    return _Test_Status();
}
