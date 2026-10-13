// (Test) Status: 0
// A test file for <_int.h> types and limits against C99's.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <_int.h>

// The types _int converts to and counts in should be C99's widest and size_t.
static void Test_IntLimits(void)
{
    _Static_assert(sizeof(long) == sizeof(intmax_t), "_int should convert to an intmax_t");
    _Static_assert(_INT_SIGNED_MAX == INTMAX_MAX, "_INT_SIGNED_MAX should be INTMAX_MAX");
    _Static_assert(sizeof(unsigned long) == sizeof(uintmax_t), "_int should convert to a uintmax_t");
    _Static_assert(_INT_VALUE_MAX == UINTMAX_MAX, "_INT_VALUE_MAX should be UINTMAX_MAX");
    _Static_assert(sizeof(int) == sizeof(wchar_t), "_int should read a wchar_t as an int");
    _Static_assert(sizeof(unsigned long) == sizeof(size_t), "_int should count in a size_t");
    _Static_assert(LONG_MAX == INTMAX_MAX && ULONG_MAX == UINTMAX_MAX, "strtol's long should be intmax_t");
    _Static_assert(LLONG_MAX == INTMAX_MAX && ULLONG_MAX == UINTMAX_MAX, "strtoll's long long should be intmax_t");
}

int main(void)
{
    Test_IntLimits();
    return _Test_Status();
}
