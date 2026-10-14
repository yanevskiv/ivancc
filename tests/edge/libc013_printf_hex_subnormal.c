// (Test) Status: 0
// (Test) Output:
// | [0x0.0000000000001p-1022] 25
// | [0x0.8p-1022|0x0.fffffffffffffp-1022] 37
// | [0x0p-1022|0x0.001p-1022] 25
// | [0x0.000000000000001p-16385] 28
// | [0x4p-16385|0x7.fffffffffffffffp-16385] 39
// Non-standard: %a should write a subnormal at the least normal value's power.
// Liberty: a value not normalized has its first digit unspecified.
// [C99:7.19.6.1p8]: printf's conversion specifiers

#include <stdio.h>
#include <float.h>

int main(void)
{
    double tiny = DBL_MIN * DBL_EPSILON;
    long double small = LDBL_MIN * LDBL_EPSILON;

    printf(" %d\n", printf("[%a]", tiny));
    printf(" %d\n", printf("[%a|%a]", DBL_MIN / 2, DBL_MIN - tiny));
    printf(" %d\n", printf("[%.0a|%.3a]", tiny, DBL_MIN / 4096));
    printf(" %d\n", printf("[%La]", small));
    printf(" %d\n", printf("[%La|%La]", LDBL_MIN / 2, LDBL_MIN - small));
    return 0;
}
