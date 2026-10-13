// (Test) Status: 0
// A test file for <_bigint.h> comparison and subtraction.

// Include helpers.
#include "_util07_00_bigint_helpers.inc"

// Numbers should compare by value, and a difference borrow and shrink.
static void Test_BigintSubtract(void)
{
    _Bigint_Number num;
    _Bigint_Number other;

    _Bigint_SetValue(&num, 5);
    _Bigint_SetValue(&other, 7);
    _TEST_TRUE(_Bigint_CompareNumbers(&num, &other) < 0);
    _TEST_TRUE(_Bigint_CompareNumbers(&other, &num) > 0);
    _TEST_INT(_Bigint_CompareNumbers(&num, &num), 0);
    _Bigint_SetValue(&num, 0x100000000ULL);
    _TEST_TRUE(_Bigint_CompareNumbers(&num, &other) > 0);
    _Bigint_SetValue(&other, 0x1FFFFFFFFULL);
    _TEST_TRUE(_Bigint_CompareNumbers(&num, &other) < 0);
    _Bigint_SetValue(&other, 0);
    _TEST_TRUE(_Bigint_CompareNumbers(&num, &other) > 0);
    _TEST_INT(_Bigint_CompareNumbers(&other, &other), 0);
    _Bigint_SubtractNumber(&num, &other);
    _TEST_TRUE(holds(&num, 2, 0, 1));
    _Bigint_SetValue(&other, 1);
    _Bigint_SubtractNumber(&num, &other);
    _TEST_TRUE(holds(&num, 1, WORD_MAX, WORD_MAX));
    _Bigint_SetValue(&num, 1);
    _Bigint_ShiftBitsLeft(&num, 96);
    _Bigint_SubtractNumber(&num, &other);
    _TEST_TRUE(holds(&num, 3, WORD_MAX, WORD_MAX));
    _TEST_INT(_Bigint_CountBits(&num), 96);
    _Bigint_SubtractNumber(&num, &num);
    _TEST_TRUE(holds(&num, 0, 0, 0));
}

int main(void)
{
    Test_BigintSubtract();
    return _Test_Status();
}
