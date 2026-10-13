// (Test) Status: 0
// A test file for <stdio.h> the length modifiers of scanf's integers.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define PAIR 2

// [C99:S7.19.6.2p11] An integer should be stored as the type its length names.
static void Test_StdioScanIntegerLengths(void)
{
    size_t z = 0;
    long l = NONE;
    intmax_t j = NONE;
    ptrdiff_t t = NONE;
    char text[HOLD];
    long long ll = NONE;
    unsigned long ul = 0;
    uintmax_t uj = 0;
    unsigned long long ull = 0;
    short h[PAIR] = {NONE, NONE};
    signed char hh[PAIR] = {NONE, NONE};
    unsigned short uh[PAIR] = {FILL, FILL};
    unsigned char uhh[PAIR] = {FILL, FILL};

    _TEST_INT(sscanf("-128 255", "%hhd%hhu", hh, uhh), 2);
    _TEST_INT(hh[0], SCHAR_MIN);
    _TEST_INT(hh[1], NONE);
    _TEST_UINT(uhh[0], UCHAR_MAX);
    _TEST_UINT(uhh[1], FILL);
    _TEST_INT(sscanf("-32768 ffff", "%hi%hx", h, uh), 2);
    _TEST_INT(h[0], SHRT_MIN);
    _TEST_INT(h[1], NONE);
    _TEST_UINT(uh[0], USHRT_MAX);
    _TEST_UINT(uh[1], FILL);
    _TEST_TRUE(sprintf(text, "%ld %lu", LONG_MIN, ULONG_MAX) > 0);
    _TEST_INT(sscanf(text, "%ld%lu", &l, &ul), 2);
    _TEST_INT(l, LONG_MIN);
    _TEST_UINT(ul, ULONG_MAX);
    _TEST_TRUE(sprintf(text, "%lld %llo", LLONG_MIN, ULLONG_MAX) > 0);
    _TEST_INT(sscanf(text, "%lld%llo", &ll, &ull), 2);
    _TEST_INT(ll, LLONG_MIN);
    _TEST_UINT(ull, ULLONG_MAX);
    _TEST_TRUE(sprintf(text, "%jd %jX", INTMAX_MAX, UINTMAX_MAX) > 0);
    _TEST_INT(sscanf(text, "%ji%jX", &j, &uj), 2);
    _TEST_INT(j, INTMAX_MAX);
    _TEST_UINT(uj, UINTMAX_MAX);
    _TEST_TRUE(sprintf(text, "%zu %td", SIZE_MAX, (ptrdiff_t) PTRDIFF_MIN) > 0);
    _TEST_INT(sscanf(text, "%zu%td", &z, &t), 2);
    _TEST_UINT(z, SIZE_MAX);
    _TEST_INT(t, PTRDIFF_MIN);
    _TEST_INT(sscanf("-1 0x7fffffffffffffff", "%lu%li", &ul, &l), 2);
    _TEST_UINT(ul, ULONG_MAX);
    _TEST_INT(l, LONG_MAX);
}

int main(void)
{
    Test_StdioScanIntegerLengths();
    return _Test_Status();
}
