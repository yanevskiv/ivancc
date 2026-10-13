// (Test) Status: 0
// A test file for <_bigint.h> multiplication by a word and by a power of 5.

// Include helpers.
#include "_util07_00_bigint_helpers.inc"

// A product should carry into as many new words as it needs.
static void Test_BigintMultiply(void)
{
    _Bigint_Number num;
    _Bigint_Number want;

    _Bigint_SetValue(&num, 0);
    _Bigint_MulAddWords(&num, 5, 7);
    _TEST_TRUE(holds(&num, 1, 7, 7));
    _Bigint_SetValue(&num, WORD_MAX);
    _Bigint_MulAddWords(&num, WORD_MAX, WORD_MAX);
    _TEST_TRUE(holds(&num, 2, 0, WORD_MAX));
    _Bigint_SetValue(&num, LONG_ALL);
    _Bigint_MulAddWords(&num, 10, 9);
    _TEST_TRUE(holds(&num, 3, WORD_MAX, 9));
    _TEST_UINT(num.bn_word[1], WORD_MAX);
    _Bigint_SetValue(&num, 3);
    _Bigint_MulPow5(&num, 0);
    _TEST_TRUE(holds(&num, 1, 3, 3));
    _Bigint_SetValue(&num, 1);
    _Bigint_MulPow5(&num, 13);
    _TEST_TRUE(holds(&num, 1, _BIGINT_POW5_WORD, _BIGINT_POW5_WORD));
    _Bigint_SetValue(&num, 1);
    _Bigint_MulPow5(&num, 27);
    _Bigint_SetValue(&want, 7450580596923828125ULL);
    _TEST_INT(_Bigint_CompareNumbers(&num, &want), 0);
    _Bigint_SetValue(&num, 1);
    _Bigint_MulPow5(&num, 100);
    _TEST_INT(_Bigint_CountBits(&num), 233);
}

int main(void)
{
    Test_BigintMultiply();
    return _Test_Status();
}
