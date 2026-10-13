// (Test) Status: 0
// A test file for <stdint.h> exact-width types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99:S7.18.1.1p1] An exact-width type should have just its bits and their limits.
static void Test_StdintExact(void)
{
    _TEST_UINT(sizeof(int8_t) * CHAR_BIT, 8);
    _TEST_UINT(sizeof(int16_t) * CHAR_BIT, 16);
    _TEST_UINT(sizeof(int32_t) * CHAR_BIT, 32);
    _TEST_UINT(sizeof(int64_t) * CHAR_BIT, 64);
    _TEST_UINT(sizeof(uint8_t) * CHAR_BIT, 8);
    _TEST_UINT(sizeof(uint16_t) * CHAR_BIT, 16);
    _TEST_UINT(sizeof(uint32_t) * CHAR_BIT, 32);
    _TEST_UINT(sizeof(uint64_t) * CHAR_BIT, 64);

    _TEST_TRUE((int8_t) -1 < 0);
    _TEST_TRUE((int16_t) -1 < 0);
    _TEST_TRUE((int32_t) -1 < 0);
    _TEST_TRUE((int64_t) -1 < 0);
    _TEST_TRUE((uint8_t) -1 > 0);
    _TEST_TRUE((uint16_t) -1 > 0);
    _TEST_TRUE((uint32_t) -1 > 0);
    _TEST_TRUE((uint64_t) -1 > 0);

    _TEST_INT(INT8_MIN, -128);
    _TEST_INT(INT8_MAX, 127);
    _TEST_INT(UINT8_MAX, 255);
    _TEST_INT(INT16_MIN, -32768);
    _TEST_INT(INT16_MAX, 32767);
    _TEST_INT(UINT16_MAX, 65535);
    _TEST_INT(INT32_MIN, -2147483647 - 1);
    _TEST_INT(INT32_MAX, 2147483647);
    _TEST_INT(UINT32_MAX, 4294967295L);
    _TEST_INT(INT64_MIN, -9223372036854775807L - 1);
    _TEST_INT(INT64_MAX, 9223372036854775807L);
    _TEST_UINT(UINT64_MAX, 18446744073709551615UL);
}

int main(void)
{
    Test_StdintExact();
    return _Test_Status();
}
