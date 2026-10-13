// (Test) Status: 0
// A test file for <_bigint.h> shifts of bits.

// Include helpers.
#include "_util07_00_bigint_helpers.inc"

// A shift left should grow a number by words, and one right drop what falls.
static void Test_BigintShift(void)
{
    _Bigint_Number num;

    _Bigint_SetValue(&num, 1);
    _Bigint_ShiftBitsLeft(&num, 0);
    _TEST_TRUE(holds(&num, 1, 1, 1));
    _Bigint_ShiftBitsLeft(&num, 32);
    _TEST_TRUE(holds(&num, 2, 0, 1));
    _Bigint_ShiftBitsLeft(&num, 68);
    _TEST_TRUE(holds(&num, 4, 0, 0x10));
    _TEST_INT(_Bigint_CountBits(&num), 101);
    _Bigint_ShiftBitsRight(&num, 100);
    _TEST_TRUE(holds(&num, 1, 1, 1));
    _Bigint_SetValue(&num, 0xFF);
    _Bigint_ShiftBitsRight(&num, 4);
    _TEST_TRUE(holds(&num, 1, 0xF, 0xF));
    _Bigint_ShiftBitsRight(&num, 0);
    _TEST_TRUE(holds(&num, 1, 0xF, 0xF));
    _Bigint_ShiftBitsRight(&num, 4);
    _TEST_TRUE(holds(&num, 0, 0, 0));
    _Bigint_SetValue(&num, LONG_ALL);
    _Bigint_ShiftBitsLeft(&num, 1);
    _TEST_TRUE(holds(&num, 3, WORD_MAX - 1, 1));
    _Bigint_ShiftBitsRight(&num, 200);
    _TEST_TRUE(holds(&num, 0, 0, 0));
    _Bigint_ShiftBitsLeft(&num, 40);
    _TEST_TRUE(holds(&num, 0, 0, 0));
}

int main(void)
{
    Test_BigintShift();
    return _Test_Status();
}
