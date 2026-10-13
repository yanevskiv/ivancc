// (Test) Status: 0
// A test file for <limits.h> limits in directives.

// Include helpers.
#include "libc04_00_limits_helpers.inc"

// [C99 S5.2.4.2.1p1] The character types' limits should be x86-64 Linux's in #if.
#define DIRECTIVE_CHAR 0
#if (CHAR_BIT == 8) \
    && (SCHAR_MIN == -128) \
    && (SCHAR_MAX == 127) \
    && (UCHAR_MAX == 255) \
    && (CHAR_MIN == -128) \
    && (CHAR_MAX == 127) \
    && (MB_LEN_MAX == 16)
#undef DIRECTIVE_CHAR
#define DIRECTIVE_CHAR 1
#endif

// [C99 S5.2.4.2.1p1] The limits of short and int should be x86-64 Linux's in #if.
#define DIRECTIVE_INT 0
#if (SHRT_MIN == -32768) \
    && (SHRT_MAX == 32767) \
    && (USHRT_MAX == 65535) \
    && (INT_MIN == -2147483647 - 1) \
    && (INT_MAX == 2147483647) \
    && (UINT_MAX == 4294967295U)
#undef DIRECTIVE_INT
#define DIRECTIVE_INT 1
#endif

// [C99 S5.2.4.2.1p1] The limits of long should be x86-64 Linux's in #if.
#define DIRECTIVE_LONG 0
#if (LONG_MIN == -9223372036854775807L - 1) \
    && (LONG_MAX == 9223372036854775807L) \
    && (ULONG_MAX == 18446744073709551615UL)
#undef DIRECTIVE_LONG
#define DIRECTIVE_LONG 1
#endif

// [C99 S5.2.4.2.1p1] The limits of long long should be x86-64 Linux's in #if.
#define DIRECTIVE_LLONG 0
#if (LLONG_MIN == -9223372036854775807LL - 1) \
    && (LLONG_MAX == 9223372036854775807LL) \
    && (ULLONG_MAX == 18446744073709551615ULL)
#undef DIRECTIVE_LLONG
#define DIRECTIVE_LLONG 1
#endif

// [C99 S5.2.4.2.1p1] Every limit should hold in a directive as in an expression.
static void Test_LimitsDirective(void)
{
    _TEST_INT(DIRECTIVE_CHAR, 1);
    _TEST_INT(DIRECTIVE_INT, 1);
    _TEST_INT(DIRECTIVE_LONG, 1);
    _TEST_INT(DIRECTIVE_LLONG, 1);
}

int main(void)
{
    Test_LimitsDirective();
    return _Test_Status();
}
