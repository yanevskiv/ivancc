// (Test) Status: 0
// A test file for <stdint.h> constant macro types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99:7.18.4.1p1] A constant macro should make a constant of its least type.
static void Test_StdintConstantTypes(void)
{
    _TEST_UINT(sizeof(INT8_C(1)), sizeof(int));
    _TEST_UINT(sizeof(INT16_C(1)), sizeof(int));
    _TEST_UINT(sizeof(INT32_C(1)), sizeof(int_least32_t));
    _TEST_UINT(sizeof(INT64_C(1)), sizeof(int_least64_t));
    _TEST_UINT(sizeof(INTMAX_C(1)), sizeof(intmax_t));
    _TEST_UINT(sizeof(UINT8_C(1)), sizeof(int));
    _TEST_UINT(sizeof(UINT16_C(1)), sizeof(int));
    _TEST_UINT(sizeof(UINT32_C(1)), sizeof(uint_least32_t));
    _TEST_UINT(sizeof(UINT64_C(1)), sizeof(uint_least64_t));
    _TEST_UINT(sizeof(UINTMAX_C(1)), sizeof(uintmax_t));

    _TEST_TRUE(INT32_C(0) - 1 < 0);
    _TEST_TRUE(INT64_C(0) - 1 < 0);
    _TEST_TRUE(INTMAX_C(0) - 1 < 0);
    _TEST_TRUE(UINT8_C(0) - 1 < 0);
    _TEST_TRUE(UINT16_C(0) - 1 < 0);
    _TEST_TRUE(UINT32_C(0) - 1 > 0);
    _TEST_TRUE(UINT64_C(0) - 1 > 0);
    _TEST_TRUE(UINTMAX_C(0) - 1 > 0);
}

int main(void)
{
    Test_StdintConstantTypes();
    return _Test_Status();
}
