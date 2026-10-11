// (Test) Status: 0
// <stdint.h> declares the exact-width, minimum-width and fastest minimum-width integer types of 8, 16, 32 and 64 bits, intptr_t and uintptr_t, which a pointer to void survives, and intmax_t and uintmax_t (S7.18.1),
// each signed one signed and each unsigned one unsigned, and defines each type's limits (S7.18.2), those its size gives a type with no padding, and the limits of ptrdiff_t, sig_atomic_t, size_t, wchar_t and wint_t (S7.18.3), those of x86-64 Linux,
// each limit a constant fit for #if and of the type its own type promotes to (S7.18.2p2), and the macros INT8_C to UINT64_C, INTMAX_C and UINTMAX_C, each making of a decimal, octal or hexadecimal constant one of its type promoted, fit for #if (S7.18.4).

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>

// Whether a directive finds the limits of the 8-bit and 16-bit types.
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

// Whether a directive finds the limits of the 32-bit and 64-bit types.
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

// Whether a directive finds the least-width types' limits wide enough.
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

// Whether a directive finds the fastest types' limits wide enough.
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

// Whether a directive finds intptr_t's and intmax_t's limits wide enough.
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

// Whether a directive finds the other integer types' limits wide enough.
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

// Whether a directive finds each constant macro giving its type's value.
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

// Return the greatest value of an unsigned type of a size, with no padding.
static unsigned long umost(size_t size)
{
    return ULONG_MAX >> ((sizeof(unsigned long) - size) * CHAR_BIT);
}

// Return the greatest value of a signed type of a size, with no padding.
static long most(size_t size)
{
    return (long) (umost(size) >> 1);
}

// Return the least value of a signed type of a size, in two's complement.
static long least(size_t size)
{
    return -most(size) - 1;
}

// An exact-width type has just its bits, and the limits those give.
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

// A minimum-width type has at least its bits, and the limits its size gives.
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

// A fastest type has at least its bits, and the limits its size gives.
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

// A pointer to void survives intptr_t and uintptr_t, whose limits fit them.
static void Test_StdintPointer(void)
{
    int number = 0;
    void *ptr = &number;
    intptr_t held = (intptr_t) ptr;
    uintptr_t uheld = (uintptr_t) ptr;

    _TEST_TRUE((void *) held == ptr);
    _TEST_TRUE((void *) uheld == ptr);
    _TEST_TRUE((intptr_t) -1 < 0);
    _TEST_TRUE((uintptr_t) -1 > 0);
    _TEST_INT(INTPTR_MIN, least(sizeof(intptr_t)));
    _TEST_INT(INTPTR_MAX, most(sizeof(intptr_t)));
    _TEST_UINT(UINTPTR_MAX, umost(sizeof(uintptr_t)));
}

// intmax_t and uintmax_t are no narrower than long long, with limits to fit.
static void Test_StdintGreatest(void)
{
    _TEST_TRUE(sizeof(intmax_t) >= sizeof(long long));
    _TEST_TRUE(sizeof(uintmax_t) >= sizeof(unsigned long long));
    _TEST_TRUE((intmax_t) -1 < 0);
    _TEST_TRUE((uintmax_t) -1 > 0);
    _TEST_INT(INTMAX_MIN, least(sizeof(intmax_t)));
    _TEST_INT(INTMAX_MAX, most(sizeof(intmax_t)));
    _TEST_UINT(UINTMAX_MAX, umost(sizeof(uintmax_t)));
    _TEST_TRUE(INTMAX_MIN <= LLONG_MIN);
    _TEST_TRUE(INTMAX_MAX >= LLONG_MAX);
    _TEST_TRUE(UINTMAX_MAX >= ULLONG_MAX);
}

// The other types' limits are those their sizes give, or x86-64 Linux's.
static void Test_StdintOther(void)
{
    _TEST_INT(PTRDIFF_MIN, least(sizeof(ptrdiff_t)));
    _TEST_INT(PTRDIFF_MAX, most(sizeof(ptrdiff_t)));
    _TEST_UINT(SIZE_MAX, umost(sizeof(size_t)));
    _TEST_UINT(SIZE_MAX, (size_t) -1);
    _TEST_TRUE((wchar_t) -1 < 0);
    _TEST_INT(WCHAR_MIN, least(sizeof(wchar_t)));
    _TEST_INT(WCHAR_MAX, most(sizeof(wchar_t)));
    _TEST_INT(SIG_ATOMIC_MIN, INT_MIN);
    _TEST_INT(SIG_ATOMIC_MAX, INT_MAX);
    _TEST_INT(WINT_MIN, 0);
    _TEST_INT(WINT_MAX, 4294967295L);
}

// A limit has the type its own type promotes to, so an unsigned one wraps.
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

// A constant macro keeps its argument's value in any of the three bases.
static void Test_StdintConstants(void)
{
    _TEST_INT(INT8_C(127), 127);
    _TEST_INT(INT8_C(0177), 127);
    _TEST_INT(INT8_C(0x7F), 127);
    _TEST_INT(INT16_C(32767), 32767);
    _TEST_INT(INT32_C(2147483647), 2147483647);
    _TEST_INT(INT64_C(9223372036854775807), INT64_MAX);
    _TEST_INT(INT64_C(0x7FFFFFFFFFFFFFFF), INT64_MAX);
    _TEST_INT(-INT64_C(9223372036854775807) - 1, INT64_MIN);
    _TEST_INT(INTMAX_C(9223372036854775807), INTMAX_MAX);
    _TEST_INT(UINT8_C(255), 255);
    _TEST_INT(UINT16_C(65535), 65535);
    _TEST_INT(UINT16_C(0xFFFF), 65535);
    _TEST_INT(UINT32_C(4294967295), 4294967295L);
    _TEST_UINT(UINT64_C(18446744073709551615), UINT64_MAX);
    _TEST_UINT(UINT64_C(01777777777777777777777), UINT64_MAX);
    _TEST_UINT(UINT64_C(0xFFFFFFFFFFFFFFFF), UINT64_MAX);
    _TEST_UINT(UINTMAX_C(18446744073709551615), UINTMAX_MAX);
}

// A constant macro makes a constant of its minimum-width type, promoted.
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

// Every limit and constant macro holds in a directive as in an expression.
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
    Test_StdintExact();
    Test_StdintLeast();
    Test_StdintFast();
    Test_StdintPointer();
    Test_StdintGreatest();
    Test_StdintOther();
    Test_StdintLimitTypes();
    Test_StdintConstants();
    Test_StdintConstantTypes();
    Test_StdintDirective();
    return _Test_Status();
}
