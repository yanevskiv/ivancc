// (Test) Status: 0
// A test file for <stdint.h> fastest minimum-width types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// A fastest type should have at least its bits.
// [C99:7.18.1.3p2]: int_fastN_t and uint_fastN_t, the fastest of width at least N
static void Test_StdintFast(void)
{
    _TEST_TRUE(sizeof(int_fast8_t) * CHAR_BIT >= 8);
    _TEST_TRUE(sizeof(int_fast16_t) * CHAR_BIT >= 16);
    _TEST_TRUE(sizeof(int_fast32_t) * CHAR_BIT >= 32);
    _TEST_TRUE(sizeof(int_fast64_t) * CHAR_BIT >= 64);
    _TEST_UINT(sizeof(uint_fast8_t), sizeof(int_fast8_t));
    _TEST_UINT(sizeof(uint_fast16_t), sizeof(int_fast16_t));
    _TEST_UINT(sizeof(uint_fast32_t), sizeof(int_fast32_t));
    _TEST_UINT(sizeof(uint_fast64_t), sizeof(int_fast64_t));

    _TEST_TRUE((int_fast8_t) -1 < 0);
    _TEST_TRUE((int_fast16_t) -1 < 0);
    _TEST_TRUE((int_fast32_t) -1 < 0);
    _TEST_TRUE((int_fast64_t) -1 < 0);
    _TEST_TRUE((uint_fast8_t) -1 > 0);
    _TEST_TRUE((uint_fast16_t) -1 > 0);
    _TEST_TRUE((uint_fast32_t) -1 > 0);
    _TEST_TRUE((uint_fast64_t) -1 > 0);

    _TEST_INT(INT_FAST8_MIN, least(sizeof(int_fast8_t)));
    _TEST_INT(INT_FAST8_MAX, most(sizeof(int_fast8_t)));
    _TEST_UINT(UINT_FAST8_MAX, umost(sizeof(uint_fast8_t)));
    _TEST_INT(INT_FAST16_MIN, least(sizeof(int_fast16_t)));
    _TEST_INT(INT_FAST16_MAX, most(sizeof(int_fast16_t)));
    _TEST_UINT(UINT_FAST16_MAX, umost(sizeof(uint_fast16_t)));
    _TEST_INT(INT_FAST32_MIN, least(sizeof(int_fast32_t)));
    _TEST_INT(INT_FAST32_MAX, most(sizeof(int_fast32_t)));
    _TEST_UINT(UINT_FAST32_MAX, umost(sizeof(uint_fast32_t)));
    _TEST_INT(INT_FAST64_MIN, least(sizeof(int_fast64_t)));
    _TEST_INT(INT_FAST64_MAX, most(sizeof(int_fast64_t)));
    _TEST_UINT(UINT_FAST64_MAX, umost(sizeof(uint_fast64_t)));
}

int main(void)
{
    Test_StdintFast();
    return _Test_Status();
}
