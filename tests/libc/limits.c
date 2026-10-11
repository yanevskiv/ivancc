// (Test) Status: 0
// <limits.h> defines CHAR_BIT, MB_LEN_MAX and the least and greatest value of each integer type (S7.10p1), those of x86-64 Linux, where char is signed, int has 32 bits and long and long long 64,
// each a constant fit for #if, and each but CHAR_BIT and MB_LEN_MAX of the type its own type promotes to (S5.2.4.2.1p1),
// UCHAR_MAX one less than 2 to the power CHAR_BIT, and char's limits those of signed char or unsigned char, whichever char behaves as (S5.2.4.2.1p2).

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>

// (S5.2.4.2.1p1) The character types' limits should be x86-64 Linux's in #if.
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

// (S5.2.4.2.1p1) The limits of short and int should be x86-64 Linux's in #if.
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

// (S5.2.4.2.1p1) The limits of long should be x86-64 Linux's in #if.
#define DIRECTIVE_LONG 0
#if (LONG_MIN == -9223372036854775807L - 1) \
    && (LONG_MAX == 9223372036854775807L) \
    && (ULONG_MAX == 18446744073709551615UL)
#undef DIRECTIVE_LONG
#define DIRECTIVE_LONG 1
#endif

// (S5.2.4.2.1p1) The limits of long long should be x86-64 Linux's in #if.
#define DIRECTIVE_LLONG 0
#if (LLONG_MIN == -9223372036854775807LL - 1) \
    && (LLONG_MAX == 9223372036854775807LL) \
    && (ULLONG_MAX == 18446744073709551615ULL)
#undef DIRECTIVE_LLONG
#define DIRECTIVE_LLONG 1
#endif

// (S5.2.4.2.1p2) A character should have 8 bits, and char its type's limits.
static void Test_LimitsChar(void)
{
    _TEST_INT(CHAR_BIT, 8);
    _TEST_INT(SCHAR_MIN, -128);
    _TEST_INT(SCHAR_MAX, 127);
    _TEST_INT(UCHAR_MAX, 255);
    _TEST_INT(UCHAR_MAX, (1 << CHAR_BIT) - 1);
    _TEST_INT(CHAR_MIN, -128);
    _TEST_INT(CHAR_MAX, 127);
    _TEST_INT(CHAR_MIN, (char) -1 < 0 ? SCHAR_MIN : 0);
    _TEST_INT(CHAR_MAX, (char) -1 < 0 ? SCHAR_MAX : UCHAR_MAX);
    _TEST_INT(MB_LEN_MAX, 16);
}

// (S5.2.4.2.1p1) short should have 16 bits and int 32.
static void Test_LimitsInt(void)
{
    _TEST_INT(SHRT_MIN, -32768);
    _TEST_INT(SHRT_MAX, 32767);
    _TEST_INT(USHRT_MAX, 65535);
    _TEST_INT(INT_MIN, -2147483647 - 1);
    _TEST_INT(INT_MAX, 2147483647);
    _TEST_INT(UINT_MAX, 4294967295L);
}

// (S5.2.4.2.1p1) long and long long should have 64 bits.
static void Test_LimitsLong(void)
{
    _TEST_INT(LONG_MIN, -9223372036854775807L - 1);
    _TEST_INT(LONG_MAX, 9223372036854775807L);
    _TEST_UINT(ULONG_MAX, 18446744073709551615UL);
    _TEST_INT(LLONG_MIN, -9223372036854775807LL - 1);
    _TEST_INT(LLONG_MAX, 9223372036854775807LL);
    _TEST_UINT(ULLONG_MAX, 18446744073709551615ULL);
}

// (S5.2.4.2.1p1) A limit should have the type its own type promotes to.
static void Test_LimitsTypes(void)
{
    _TEST_UINT(sizeof(SCHAR_MIN), sizeof(int));
    _TEST_UINT(sizeof(SCHAR_MAX), sizeof(int));
    _TEST_UINT(sizeof(UCHAR_MAX), sizeof(int));
    _TEST_UINT(sizeof(CHAR_MIN), sizeof(int));
    _TEST_UINT(sizeof(CHAR_MAX), sizeof(int));
    _TEST_UINT(sizeof(SHRT_MIN), sizeof(int));
    _TEST_UINT(sizeof(SHRT_MAX), sizeof(int));
    _TEST_UINT(sizeof(USHRT_MAX), sizeof(int));
    _TEST_UINT(sizeof(INT_MIN), sizeof(int));
    _TEST_UINT(sizeof(INT_MAX), sizeof(int));
    _TEST_UINT(sizeof(UINT_MAX), sizeof(unsigned int));
    _TEST_UINT(sizeof(LONG_MIN), sizeof(long));
    _TEST_UINT(sizeof(LONG_MAX), sizeof(long));
    _TEST_UINT(sizeof(ULONG_MAX), sizeof(unsigned long));
    _TEST_UINT(sizeof(LLONG_MIN), sizeof(long long));
    _TEST_UINT(sizeof(LLONG_MAX), sizeof(long long));
    _TEST_UINT(sizeof(ULLONG_MAX), sizeof(unsigned long long));

    _TEST_TRUE(SCHAR_MIN < 0);
    _TEST_TRUE(SHRT_MIN < 0);
    _TEST_TRUE(INT_MIN < 0);
    _TEST_TRUE(LONG_MIN < 0);
    _TEST_TRUE(LLONG_MIN < 0);
    _TEST_INT(UCHAR_MAX + 1, 256);
    _TEST_INT(USHRT_MAX + 1, 65536);
    _TEST_INT(UINT_MAX + 1, 0);
    _TEST_UINT(ULONG_MAX + 1, 0);
    _TEST_UINT(ULLONG_MAX + 1, 0);
}

// (S5.2.4.2.1p1) An object of each type should hold both of its limits.
static void Test_LimitsObjects(void)
{
    int ints[2] = {INT_MIN, INT_MAX};
    char chars[2] = {CHAR_MIN, CHAR_MAX};
    long longs[2] = {LONG_MIN, LONG_MAX};
    short shorts[2] = {SHRT_MIN, SHRT_MAX};
    unsigned int uint = UINT_MAX;
    long long llongs[2] = {LLONG_MIN, LLONG_MAX};
    signed char schars[2] = {SCHAR_MIN, SCHAR_MAX};
    unsigned char uchar = UCHAR_MAX;
    unsigned long ulong = ULONG_MAX;
    unsigned short ushort = USHRT_MAX;
    unsigned long long ullong = ULLONG_MAX;

    _TEST_INT(schars[0], SCHAR_MIN);
    _TEST_INT(schars[1], SCHAR_MAX);
    _TEST_INT(uchar, UCHAR_MAX);
    _TEST_INT(chars[0], CHAR_MIN);
    _TEST_INT(chars[1], CHAR_MAX);
    _TEST_INT(shorts[0], SHRT_MIN);
    _TEST_INT(shorts[1], SHRT_MAX);
    _TEST_INT(ushort, USHRT_MAX);
    _TEST_INT(ints[0], INT_MIN);
    _TEST_INT(ints[1], INT_MAX);
    _TEST_INT(uint, UINT_MAX);
    _TEST_INT(longs[0], LONG_MIN);
    _TEST_INT(longs[1], LONG_MAX);
    _TEST_UINT(ulong, ULONG_MAX);
    _TEST_INT(llongs[0], LLONG_MIN);
    _TEST_INT(llongs[1], LLONG_MAX);
    _TEST_UINT(ullong, ULLONG_MAX);
}

// (S5.2.4.2.1p1) Every limit should hold in a directive as in an expression.
static void Test_LimitsDirective(void)
{
    _TEST_INT(DIRECTIVE_CHAR, 1);
    _TEST_INT(DIRECTIVE_INT, 1);
    _TEST_INT(DIRECTIVE_LONG, 1);
    _TEST_INT(DIRECTIVE_LLONG, 1);
}

int main(void)
{
    Test_LimitsChar();
    Test_LimitsInt();
    Test_LimitsLong();
    Test_LimitsTypes();
    Test_LimitsObjects();
    Test_LimitsDirective();
    return _Test_Status();
}
