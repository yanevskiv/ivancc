// (Test) Status: 0
// A test file for <stdint.h> minimum-width types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99:S7.18.1.2p1] A minimum-width type should have at least its bits.
static void Test_StdintLeast(void)
{
    _TEST_TRUE(sizeof(int_least8_t) * CHAR_BIT >= 8);
    _TEST_TRUE(sizeof(int_least16_t) * CHAR_BIT >= 16);
    _TEST_TRUE(sizeof(int_least32_t) * CHAR_BIT >= 32);
    _TEST_TRUE(sizeof(int_least64_t) * CHAR_BIT >= 64);
    _TEST_UINT(sizeof(uint_least8_t), sizeof(int_least8_t));
    _TEST_UINT(sizeof(uint_least16_t), sizeof(int_least16_t));
    _TEST_UINT(sizeof(uint_least32_t), sizeof(int_least32_t));
    _TEST_UINT(sizeof(uint_least64_t), sizeof(int_least64_t));

    _TEST_TRUE((int_least8_t) -1 < 0);
    _TEST_TRUE((int_least16_t) -1 < 0);
    _TEST_TRUE((int_least32_t) -1 < 0);
    _TEST_TRUE((int_least64_t) -1 < 0);
    _TEST_TRUE((uint_least8_t) -1 > 0);
    _TEST_TRUE((uint_least16_t) -1 > 0);
    _TEST_TRUE((uint_least32_t) -1 > 0);
    _TEST_TRUE((uint_least64_t) -1 > 0);

    _TEST_INT(INT_LEAST8_MIN, least(sizeof(int_least8_t)));
    _TEST_INT(INT_LEAST8_MAX, most(sizeof(int_least8_t)));
    _TEST_UINT(UINT_LEAST8_MAX, umost(sizeof(uint_least8_t)));
    _TEST_INT(INT_LEAST16_MIN, least(sizeof(int_least16_t)));
    _TEST_INT(INT_LEAST16_MAX, most(sizeof(int_least16_t)));
    _TEST_UINT(UINT_LEAST16_MAX, umost(sizeof(uint_least16_t)));
    _TEST_INT(INT_LEAST32_MIN, least(sizeof(int_least32_t)));
    _TEST_INT(INT_LEAST32_MAX, most(sizeof(int_least32_t)));
    _TEST_UINT(UINT_LEAST32_MAX, umost(sizeof(uint_least32_t)));
    _TEST_INT(INT_LEAST64_MIN, least(sizeof(int_least64_t)));
    _TEST_INT(INT_LEAST64_MAX, most(sizeof(int_least64_t)));
    _TEST_UINT(UINT_LEAST64_MAX, umost(sizeof(uint_least64_t)));
}

int main(void)
{
    Test_StdintLeast();
    return _Test_Status();
}
