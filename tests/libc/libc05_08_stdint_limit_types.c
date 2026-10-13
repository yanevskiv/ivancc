// (Test) Status: 0
// A test file for <stdint.h> limit types.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99 S7.18.2p2] A limit should have the type its own type promotes to.
static void Test_StdintLimitTypes(void)
{
    _TEST_UINT(sizeof(INT8_MIN), sizeof(int));
    _TEST_UINT(sizeof(INT8_MAX), sizeof(int));
    _TEST_UINT(sizeof(UINT8_MAX), sizeof(int));
    _TEST_UINT(sizeof(INT16_MIN), sizeof(int));
    _TEST_UINT(sizeof(INT16_MAX), sizeof(int));
    _TEST_UINT(sizeof(UINT16_MAX), sizeof(int));
    _TEST_UINT(sizeof(INT32_MIN), sizeof(int32_t));
    _TEST_UINT(sizeof(INT32_MAX), sizeof(int32_t));
    _TEST_UINT(sizeof(UINT32_MAX), sizeof(uint32_t));
    _TEST_UINT(sizeof(INT64_MIN), sizeof(int64_t));
    _TEST_UINT(sizeof(INT64_MAX), sizeof(int64_t));
    _TEST_UINT(sizeof(UINT64_MAX), sizeof(uint64_t));
    _TEST_UINT(sizeof(INT_LEAST64_MAX), sizeof(int_least64_t));
    _TEST_UINT(sizeof(UINT_LEAST64_MAX), sizeof(uint_least64_t));
    _TEST_UINT(sizeof(INT_FAST64_MAX), sizeof(int_fast64_t));
    _TEST_UINT(sizeof(UINT_FAST64_MAX), sizeof(uint_fast64_t));
    _TEST_UINT(sizeof(INTPTR_MAX), sizeof(intptr_t));
    _TEST_UINT(sizeof(UINTPTR_MAX), sizeof(uintptr_t));
    _TEST_UINT(sizeof(INTMAX_MAX), sizeof(intmax_t));
    _TEST_UINT(sizeof(UINTMAX_MAX), sizeof(uintmax_t));
    _TEST_UINT(sizeof(PTRDIFF_MAX), sizeof(ptrdiff_t));
    _TEST_UINT(sizeof(SIZE_MAX), sizeof(size_t));
    _TEST_UINT(sizeof(WCHAR_MAX), sizeof(wchar_t));

    _TEST_TRUE(INT8_MIN < 0);
    _TEST_TRUE(INT16_MIN < 0);
    _TEST_TRUE(INT32_MIN < 0);
    _TEST_TRUE(INT64_MIN < 0);
    _TEST_INT(UINT8_MAX + 1, 256);
    _TEST_INT(UINT16_MAX + 1, 65536);
    _TEST_INT(UINT32_MAX + 1, 0);
    _TEST_UINT(UINT64_MAX + 1, 0);
    _TEST_UINT(UINTPTR_MAX + 1, 0);
    _TEST_UINT(UINTMAX_MAX + 1, 0);
    _TEST_UINT(SIZE_MAX + 1, 0);
    _TEST_INT(WINT_MAX + 1, 0);
    _TEST_TRUE(WINT_MIN - 1 > 0);
}

int main(void)
{
    Test_StdintLimitTypes();
    return _Test_Status();
}
