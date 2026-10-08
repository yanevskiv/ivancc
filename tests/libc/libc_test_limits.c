// (Test) Status: 0
// <limits.h> gives the limits of the integer types on x86-64 Linux (S7.10).

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>

int main(void)
{
    if (CHAR_BIT != 8) return 1;
    if (SCHAR_MIN != -128 or SCHAR_MAX != 127 or UCHAR_MAX != 255) return 2;
    if (CHAR_MIN != -128 or CHAR_MAX != 127) return 3;
    if (MB_LEN_MAX != 16) return 4;
    if (SHRT_MIN != -32768 or SHRT_MAX != 32767 or USHRT_MAX != 65535) return 5;
    if (INT_MIN != -2147483647 - 1 or INT_MAX != 2147483647) return 6;
    if (UINT_MAX != 4294967295U) return 7;
    if (LONG_MAX != 9223372036854775807L or LONG_MIN != -9223372036854775807L - 1) return 8;
    if (ULONG_MAX != 18446744073709551615UL) return 9;
    if (LLONG_MAX != 9223372036854775807LL or LLONG_MIN != -9223372036854775807LL - 1) return 10;
    if (ULLONG_MAX != 18446744073709551615ULL) return 11;
    if (INT_MIN >= 0 or INT_MAX + 0LL != 2147483647LL) return 12;
#if INT_MAX != 2147483647 || LONG_MAX != 9223372036854775807L || UINT_MAX != 4294967295U
    return 13;
#endif
    return 0;
}
