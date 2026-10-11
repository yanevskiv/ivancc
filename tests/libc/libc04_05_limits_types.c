// (Test) Status: 0
// A test file for <limits.h> limit types.

// Include helpers.
#include "libc04_00_limits_helpers.inc"

// (S5.2.4.2.1p1) A limit should have the type its own type promotes to.
static void Test_LimitsTypes(void)
{
    _TEST_UINT(sizeof(SCHAR_MIN), sizeof(int));
    _TEST_UINT(sizeof(SCHAR_MAX), sizeof(int));
    _TEST_UINT(sizeof(UCHAR_MAX), sizeof(int));
    _TEST_UINT(sizeof(CHAR_MIN), sizeof(int));
    _TEST_UINT(sizeof(CHAR_MAX), sizeof(int));
    _TEST_UINT(sizeof(SHRT_MIN), sizeof(int));
    _TEST_UINT(sizeof(SHRT_MAX), sizeof(int));
    _TEST_UINT(sizeof(USHRT_MAX), sizeof(int));
    _TEST_UINT(sizeof(INT_MIN), sizeof(int));
    _TEST_UINT(sizeof(INT_MAX), sizeof(int));
    _TEST_UINT(sizeof(UINT_MAX), sizeof(unsigned int));
    _TEST_UINT(sizeof(LONG_MIN), sizeof(long));
    _TEST_UINT(sizeof(LONG_MAX), sizeof(long));
    _TEST_UINT(sizeof(ULONG_MAX), sizeof(unsigned long));
    _TEST_UINT(sizeof(LLONG_MIN), sizeof(long long));
    _TEST_UINT(sizeof(LLONG_MAX), sizeof(long long));
    _TEST_UINT(sizeof(ULLONG_MAX), sizeof(unsigned long long));

    _TEST_TRUE(SCHAR_MIN < 0);
    _TEST_TRUE(SHRT_MIN < 0);
    _TEST_TRUE(INT_MIN < 0);
    _TEST_TRUE(LONG_MIN < 0);
    _TEST_TRUE(LLONG_MIN < 0);
    _TEST_INT(UCHAR_MAX + 1, 256);
    _TEST_INT(USHRT_MAX + 1, 65536);
    _TEST_INT(UINT_MAX + 1, 0);
    _TEST_UINT(ULONG_MAX + 1, 0);
    _TEST_UINT(ULLONG_MAX + 1, 0);
}

int main(void)
{
    Test_LimitsTypes();
    return _Test_Status();
}
