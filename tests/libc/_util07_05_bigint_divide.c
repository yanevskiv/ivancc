// (Test) Status: 0
// A test file for <_bigint.h> division and splitting.

// Include helpers.
#include "_util07_00_bigint_helpers.inc"

// A division should give its quotient and leave or return its remainder.
static void Test_BigintDivide(void)
{
    _Bigint_Number num;
    _Bigint_Number den;

    _Bigint_SetValue(&num, 1000000007);
    _TEST_UINT(_Bigint_DivideWord(&num, 10), 7);
    _TEST_TRUE(holds(&num, 1, 100000000, 100000000));
    _Bigint_SetValue(&num, LONG_ALL);
    _TEST_UINT(_Bigint_DivideWord(&num, _BIGINT_POW10_WORD), 709551615);
    _TEST_TRUE(holds(&num, 2, 1266874889, 4));
    _Bigint_SetValue(&num, 3);
    _TEST_UINT(_Bigint_DivideWord(&num, 5), 3);
    _TEST_TRUE(holds(&num, 0, 0, 0));
    _Bigint_SetValue(&num, 1000);
    _Bigint_SetValue(&den, 7);
    _TEST_UINT(_Bigint_DivideNumber(&num, &den), 142);
    _TEST_TRUE(holds(&num, 1, 6, 6));
    _TEST_UINT(_Bigint_DivideNumber(&num, &den), 0);
    _TEST_TRUE(holds(&num, 1, 6, 6));
    _Bigint_SetValue(&num, 1);
    _Bigint_ShiftBitsLeft(&num, 100);
    _Bigint_MulAddWords(&num, 1, 5);
    _Bigint_SetValue(&den, 1);
    _Bigint_ShiftBitsLeft(&den, 40);
    _TEST_UINT(_Bigint_DivideNumber(&num, &den), 1ULL << 60);
    _TEST_TRUE(holds(&num, 1, 5, 5));
    _Bigint_SetValue(&num, 0x123456789ABCDEF0ULL);
    _TEST_UINT(_Bigint_SplitBits(&num, 16), 0x123456789ABCULL);
    _TEST_TRUE(holds(&num, 1, 0xDEF0, 0xDEF0));
    _TEST_UINT(_Bigint_SplitBits(&num, 64), 0);
    _TEST_TRUE(holds(&num, 1, 0xDEF0, 0xDEF0));
    _Bigint_SetValue(&num, 0x123456789ABCDEF0ULL);
    _TEST_UINT(_Bigint_SplitBits(&num, 32), 0x12345678);
    _TEST_TRUE(holds(&num, 1, 0x9ABCDEF0U, 0x9ABCDEF0U));
    _Bigint_SetValue(&num, 0x100000000ULL);
    _TEST_UINT(_Bigint_SplitBits(&num, 8), 0x1000000);
    _TEST_TRUE(holds(&num, 0, 0, 0));
}

int main(void)
{
    Test_BigintDivide();
    return _Test_Status();
}
