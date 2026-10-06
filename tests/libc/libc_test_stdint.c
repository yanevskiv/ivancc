// (Test) Status: 0
// <stdint.h> declares the integer types, their limits and constants, 7.18.

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>

int main(void)
{
    if (sizeof(int8_t) != 1 or sizeof(int16_t) != 2 or sizeof(int32_t) != 4 or sizeof(int64_t) != 8) return 1;
    if (sizeof(uint8_t) != 1 or sizeof(uint16_t) != 2 or sizeof(uint32_t) != 4 or sizeof(uint64_t) != 8) return 2;
    if ((int8_t) -1 >= 0 or (int16_t) -1 >= 0 or (int32_t) -1 >= 0 or (int64_t) -1 >= 0) return 3;
    if ((uint8_t) -1 <= 0 or (uint16_t) -1 <= 0 or (uint32_t) -1 <= 0 or (uint64_t) -1 <= 0) return 4;
    if (sizeof(int_least8_t) < 1 or sizeof(int_least16_t) < 2 or sizeof(int_least32_t) < 4 or sizeof(int_least64_t) < 8) return 5;
    if (sizeof(uint_fast8_t) < 1 or sizeof(uint_fast16_t) < 2 or sizeof(uint_fast32_t) < 4 or sizeof(uint_fast64_t) < 8) return 6;
    if (sizeof(intptr_t) != sizeof(void *) or sizeof(uintptr_t) != sizeof(void *)) return 7;
    if (sizeof(intmax_t) < sizeof(long long) or (uintmax_t) -1 <= 0) return 8;
    if (INT8_MIN != -128 or INT8_MAX != 127 or UINT8_MAX != 255) return 9;
    if (INT16_MIN != -32768 or INT16_MAX != 32767 or UINT16_MAX != 65535) return 10;
    if (INT32_MIN != -2147483647 - 1 or INT32_MAX != 2147483647 or UINT32_MAX != 4294967295U) return 11;
    if (INT64_MIN != -9223372036854775807LL - 1 or INT64_MAX != 9223372036854775807LL) return 12;
    if (UINT64_MAX != 18446744073709551615ULL) return 13;
    if (INT_LEAST8_MAX != 127 or INT_LEAST16_MAX != 32767 or INT_LEAST32_MAX != 2147483647) return 14;
    if (INT_LEAST64_MAX != 9223372036854775807LL or UINT_LEAST64_MAX != 18446744073709551615ULL) return 15;
    if (INT_FAST8_MAX != 127 or UINT_FAST8_MAX != 255) return 16;
    if (INT_FAST16_MIN != INT_FAST16_MIN or INT_FAST64_MAX != 9223372036854775807LL) return 17;
    if (INTPTR_MAX != 9223372036854775807LL or UINTPTR_MAX != 18446744073709551615ULL) return 18;
    if (INTMAX_MAX != 9223372036854775807LL or UINTMAX_MAX != 18446744073709551615ULL) return 19;
    if (INTMAX_MIN != -9223372036854775807LL - 1) return 20;
    if (PTRDIFF_MAX != 9223372036854775807LL or PTRDIFF_MIN != -9223372036854775807LL - 1) return 21;
    if (SIZE_MAX != 18446744073709551615ULL) return 22;
    if (SIG_ATOMIC_MIN != INT_MIN or SIG_ATOMIC_MAX != INT_MAX) return 23;
    if (WCHAR_MIN != INT_MIN or WCHAR_MAX != INT_MAX) return 24;
    if (WINT_MIN != 0 or WINT_MAX != 4294967295U) return 25;
    if (INT8_C(7) != 7 or INT16_C(7) != 7 or INT32_C(7) != 7 or INT64_C(7) != 7) return 26;
    if (UINT8_C(7) != 7 or UINT16_C(7) != 7 or UINT32_C(7) != 7 or UINT64_C(7) != 7) return 27;
    if (sizeof(INT64_C(1)) < 8 or sizeof(UINT64_C(1)) < 8) return 28;
    if (sizeof(INTMAX_C(1)) < 8 or sizeof(UINTMAX_C(1)) < 8) return 29;
    if (UINT32_C(0) - 1 <= 0) return 30;
#if INT32_MAX != 2147483647 || UINT64_MAX != 18446744073709551615UL || INT64_MIN >= 0
    return 31;
#endif
    return 0;
}
