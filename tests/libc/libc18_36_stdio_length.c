// (Test) Status: 0
// A test file for <stdio.h> the length modifiers on integers.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// A length modifier should give an integer argument's type.
// [C99:7.19.6.1p7]: printf's length modifiers
static void Test_StdioLength(void)
{
    _TEST_TRUE(prints("65", "%hhd", 65));
    _TEST_TRUE(prints("-1", "%hhd", -1));
    _TEST_TRUE(prints("127", "%hhd", SCHAR_MAX));
    _TEST_TRUE(prints("-127", "%hhi", -SCHAR_MAX));
    _TEST_TRUE(prints("255", "%hhu", UCHAR_MAX));
    _TEST_TRUE(prints("0", "%hhu", UCHAR_MAX + 1));
    _TEST_TRUE(prints("1", "%hhu", UCHAR_MAX + 2));
    _TEST_TRUE(prints("ff", "%hhx", UCHAR_MAX));
    _TEST_TRUE(prints("FF", "%hhX", -1));
    _TEST_TRUE(prints("377", "%hho", -1));

    _TEST_TRUE(prints("32767", "%hd", SHRT_MAX));
    _TEST_TRUE(prints("-32767", "%hd", -SHRT_MAX));
    _TEST_TRUE(prints("-1", "%hi", -1));
    _TEST_TRUE(prints("65535", "%hu", USHRT_MAX));
    _TEST_TRUE(prints("0", "%hu", USHRT_MAX + 1));
    _TEST_TRUE(prints("ffff", "%hx", -1));
    _TEST_TRUE(prints("177777", "%ho", -1));

    _TEST_TRUE(prints("2147483647", "%ld", 2147483647L));
    _TEST_TRUE(prints("-2147483647", "%li", -2147483647L));
    _TEST_TRUE(prints("4294967295", "%lu", 4294967295UL));
    _TEST_TRUE(prints("ffffffff", "%lx", 4294967295UL));
    _TEST_TRUE(prints("FFFFFFFF", "%lX", 4294967295UL));
    _TEST_TRUE(prints("37777777777", "%lo", 4294967295UL));

    _TEST_TRUE(prints("9223372036854775807", "%lld", LLONG_MAX));
    _TEST_TRUE(prints("-9223372036854775808", "%lld", LLONG_MIN));
    _TEST_TRUE(prints("-9223372036854775807", "%lli", -LLONG_MAX));
    _TEST_TRUE(prints("18446744073709551615", "%llu", ULLONG_MAX));
    _TEST_TRUE(prints("ffffffffffffffff", "%llx", ULLONG_MAX));
    _TEST_TRUE(prints("FFFFFFFFFFFFFFFF", "%llX", ULLONG_MAX));
    _TEST_TRUE(prints("1777777777777777777777", "%llo", ULLONG_MAX));
    _TEST_TRUE(prints("0x123456789abcdef0", "%#llx", 0x123456789ABCDEF0ULL));

    _TEST_TRUE(prints("9223372036854775807", "%jd", (intmax_t) INTMAX_MAX));
    _TEST_TRUE(prints("-9223372036854775808", "%jd", (intmax_t) INTMAX_MIN));
    _TEST_TRUE(prints("18446744073709551615", "%ju", (uintmax_t) UINTMAX_MAX));
    _TEST_TRUE(prints("ffffffffffffffff", "%jx", (uintmax_t) UINTMAX_MAX));

    _TEST_TRUE(prints("65535", "%zu", (size_t) 65535));
    _TEST_TRUE(prints("ffff", "%zx", (size_t) 65535));
    _TEST_TRUE(prints("8", "%zu", sizeof(long long)));
    _TEST_TRUE(prints("65535", "%td", (ptrdiff_t) 65535));
    _TEST_TRUE(prints("-65535", "%td", (ptrdiff_t) -65535));
    _TEST_TRUE(prints("-3", "%ti", (ptrdiff_t) (wide - (wide + 3))));

    _TEST_TRUE(prints("-1 -1 -1 -1", "%hhd %hd %ld %lld", -1, -1, -1L, -1LL));
    _TEST_TRUE(prints("  +00042", "%+8.5ld", 42L));
    _TEST_TRUE(prints("0000002a", "%08llx", 42ULL));
    _TEST_TRUE(prints("42   |", "%-5zu|", (size_t) 42));
}

int main(void)
{
    Test_StdioLength();
    return _Test_Status();
}
