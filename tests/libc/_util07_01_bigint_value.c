// (Test) Status: 0
// A test file for <_bigint.h> values set and their bits counted.

// Include helpers.
#include "_util07_00_bigint_helpers.inc"

// A value should take the words it needs and no more, zero taking none.
static void Test_BigintValue(void)
{
    _Bigint_Number num;

    _Bigint_SetValue(&num, 0);
    _TEST_TRUE(holds(&num, 0, 0, 0));
    _TEST_INT(_Bigint_CountBits(&num), 0);
    _Bigint_SetValue(&num, 1);
    _TEST_TRUE(holds(&num, 1, 1, 1));
    _TEST_INT(_Bigint_CountBits(&num), 1);
    _Bigint_SetValue(&num, WORD_MAX);
    _TEST_TRUE(holds(&num, 1, WORD_MAX, WORD_MAX));
    _TEST_INT(_Bigint_CountBits(&num), 32);
    _Bigint_SetValue(&num, 0x100000000ULL);
    _TEST_TRUE(holds(&num, 2, 0, 1));
    _TEST_INT(_Bigint_CountBits(&num), 33);
    _Bigint_SetValue(&num, LONG_ALL);
    _TEST_TRUE(holds(&num, 2, WORD_MAX, WORD_MAX));
    _TEST_INT(_Bigint_CountBits(&num), 64);
}

int main(void)
{
    Test_BigintValue();
    return _Test_Status();
}
