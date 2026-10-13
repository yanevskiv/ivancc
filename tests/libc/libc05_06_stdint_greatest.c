// (Test) Status: 0
// A test file for <stdint.h> intmax_t and uintmax_t types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99:S7.18.1.5p1] intmax_t and uintmax_t should be no narrower than long long.
static void Test_StdintGreatest(void)
{
    _TEST_TRUE(sizeof(intmax_t) >= sizeof(long long));
    _TEST_TRUE(sizeof(uintmax_t) >= sizeof(unsigned long long));
    _TEST_TRUE((intmax_t) -1 < 0);
    _TEST_TRUE((uintmax_t) -1 > 0);
    _TEST_INT(INTMAX_MIN, least(sizeof(intmax_t)));
    _TEST_INT(INTMAX_MAX, most(sizeof(intmax_t)));
    _TEST_UINT(UINTMAX_MAX, umost(sizeof(uintmax_t)));
    _TEST_TRUE(INTMAX_MIN <= LLONG_MIN);
    _TEST_TRUE(INTMAX_MAX >= LLONG_MAX);
    _TEST_TRUE(UINTMAX_MAX >= ULLONG_MAX);
}

int main(void)
{
    Test_StdintGreatest();
    return _Test_Status();
}
