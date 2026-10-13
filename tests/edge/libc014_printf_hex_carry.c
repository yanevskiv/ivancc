// (Test) Status: 0
// (Test) Output:
// | [0x2p+0|0x1p+1|0x1p+0] 22
// | [0x1.0p+0|0x1.2p+0|0x2.0p+0] 28
// | [0x2p+1023|0x2.00000p+1023] 27
// | [0x1p-1022] 11
// | [0x1p+4|0xep+0|0x1.0p+4] 24
// | [0x1p+16384] 12
// Non-standard: %a should keep its power when rounding carries into its digit.
// Liberty: [C99 S7.19.6.1p8] which nonzero digit comes first is unspecified.

#include <stdio.h>
#include <float.h>

int main(void)
{
    double tiny = DBL_MIN * DBL_EPSILON;

    printf(" %d\n", printf("[%.0a|%.0a|%.0a]", 1.5, 2.5, 1.4375));
    printf(" %d\n", printf("[%.1a|%.1a|%.1a]", 1.03125, 1.09375, 1.96875));
    printf(" %d\n", printf("[%.0a|%.5a]", DBL_MAX, DBL_MAX));
    printf(" %d\n", printf("[%.0a]", DBL_MIN - tiny));
    printf(" %d\n", printf("[%.0La|%.0La|%.1La]", 15.5L, 14.5L, 15.96875L));
    printf(" %d\n", printf("[%.0La]", LDBL_MAX));
    return 0;
}
