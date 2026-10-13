// (Test) Status: 0
// A test file for <stdint.h> limits and constants in directives.

// Include helpers.
#include "libc05_00_stdint_helpers.inc"

// [C99 S7.18.2p2] The 8-bit and 16-bit types' limits should be exact in #if.
#define DIRECTIVE_NARROW 0
#if (INT8_MIN == -128) \
    && (INT8_MAX == 127) \
    && (UINT8_MAX == 255) \
    && (INT16_MIN == -32768) \
    && (INT16_MAX == 32767) \
    && (UINT16_MAX == 65535)
#undef DIRECTIVE_NARROW
#define DIRECTIVE_NARROW 1
#endif

// [C99 S7.18.2p2] The 32-bit and 64-bit types' limits should be exact in #if.
#define DIRECTIVE_WIDE 0
#if (INT32_MIN == -2147483647 - 1) \
    && (INT32_MAX == 2147483647) \
    && (UINT32_MAX == 4294967295U) \
    && (INT64_MIN == -9223372036854775807 - 1) \
    && (INT64_MAX == 9223372036854775807) \
    && (UINT64_MAX == 18446744073709551615U)
#undef DIRECTIVE_WIDE
#define DIRECTIVE_WIDE 1
#endif

// [C99 S7.18.2p2] The least-width types' limits should be wide enough in #if.
#define DIRECTIVE_LEAST 0
#if (INT_LEAST8_MIN <= -127) \
    && (INT_LEAST8_MAX >= 127) \
    && (UINT_LEAST8_MAX >= 255) \
    && (INT_LEAST16_MIN <= -32767) \
    && (INT_LEAST16_MAX >= 32767) \
    && (UINT_LEAST16_MAX >= 65535) \
    && (INT_LEAST32_MIN <= -2147483647) \
    && (INT_LEAST32_MAX >= 2147483647) \
    && (UINT_LEAST32_MAX >= 4294967295U) \
    && (INT_LEAST64_MIN <= -9223372036854775807) \
    && (INT_LEAST64_MAX >= 9223372036854775807) \
    && (UINT_LEAST64_MAX >= 18446744073709551615U)
#undef DIRECTIVE_LEAST
#define DIRECTIVE_LEAST 1
#endif

// [C99 S7.18.2p2] The fastest types' limits should be wide enough in #if.
#define DIRECTIVE_FAST 0
#if (INT_FAST8_MIN <= -127) \
    && (INT_FAST8_MAX >= 127) \
    && (UINT_FAST8_MAX >= 255) \
    && (INT_FAST16_MIN <= -32767) \
    && (INT_FAST16_MAX >= 32767) \
    && (UINT_FAST16_MAX >= 65535) \
    && (INT_FAST32_MIN <= -2147483647) \
    && (INT_FAST32_MAX >= 2147483647) \
    && (UINT_FAST32_MAX >= 4294967295U) \
    && (INT_FAST64_MIN <= -9223372036854775807) \
    && (INT_FAST64_MAX >= 9223372036854775807) \
    && (UINT_FAST64_MAX >= 18446744073709551615U)
#undef DIRECTIVE_FAST
#define DIRECTIVE_FAST 1
#endif

// [C99 S7.18.2p2] intptr_t's and intmax_t's limits should be wide enough in #if.
#define DIRECTIVE_GREATEST 0
#if (INTPTR_MIN <= -32767) \
    && (INTPTR_MAX >= 32767) \
    && (UINTPTR_MAX >= 65535) \
    && (INTMAX_MIN <= -9223372036854775807) \
    && (INTMAX_MAX >= 9223372036854775807) \
    && (UINTMAX_MAX >= 18446744073709551615U)
#undef DIRECTIVE_GREATEST
#define DIRECTIVE_GREATEST 1
#endif

// [C99 S7.18.3p2] The other integer types' limits should be wide enough in #if.
#define DIRECTIVE_OTHER 0
#if (PTRDIFF_MIN <= -65535) \
    && (PTRDIFF_MAX >= 65535) \
    && (SIG_ATOMIC_MIN <= -127) \
    && (SIG_ATOMIC_MAX >= 127) \
    && (SIZE_MAX >= 65535) \
    && (WCHAR_MIN <= 0) \
    && (WCHAR_MAX >= 127) \
    && (WINT_MIN <= 0) \
    && (WINT_MAX >= 32767)
#undef DIRECTIVE_OTHER
#define DIRECTIVE_OTHER 1
#endif

// [C99 S7.18.4p3] Each constant macro should give its argument's value in #if.
#define DIRECTIVE_CONSTANT 0
#if (INT8_C(127) == 127) \
    && (INT16_C(32767) == 32767) \
    && (INT32_C(2147483647) == 2147483647) \
    && (INT64_C(9223372036854775807) == INT64_MAX) \
    && (INTMAX_C(9223372036854775807) == INTMAX_MAX) \
    && (UINT8_C(255) == 255) \
    && (UINT16_C(65535) == 65535) \
    && (UINT32_C(4294967295) == 4294967295U) \
    && (UINT64_C(18446744073709551615) == UINT64_MAX) \
    && (UINTMAX_C(18446744073709551615) == UINTMAX_MAX)
#undef DIRECTIVE_CONSTANT
#define DIRECTIVE_CONSTANT 1
#endif

// [C99 S7.18.2p2] Every limit and constant macro should hold in a directive too.
static void Test_StdintDirective(void)
{
    _TEST_INT(DIRECTIVE_NARROW, 1);
    _TEST_INT(DIRECTIVE_WIDE, 1);
    _TEST_INT(DIRECTIVE_LEAST, 1);
    _TEST_INT(DIRECTIVE_FAST, 1);
    _TEST_INT(DIRECTIVE_GREATEST, 1);
    _TEST_INT(DIRECTIVE_OTHER, 1);
    _TEST_INT(DIRECTIVE_CONSTANT, 1);
}

int main(void)
{
    Test_StdintDirective();
    return _Test_Status();
}
