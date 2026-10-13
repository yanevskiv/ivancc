// (Test) Status: 0
// (Test) Output:
// | [0x1p+0|0x1p-1|0x1.cp+0] 24
// | [0x1.0000000000001p+0] 22
// | [0x0p+0|-0x0p+0] 16
// | [0x1.p+0|0x0.p+0] 17
// | [0x8.000000000000001p-3] 24
// Non-standard: %a with no precision should write the fewest digits it needs.
// Liberty: [C99 S7.19.6.1p8] any precision that is exact is allowed.

#include <stdio.h>
#include <float.h>

int main(void)
{
    printf(" %d\n", printf("[%a|%a|%a]", 1.0, 0.5, 1.75));
    printf(" %d\n", printf("[%a]", 1.0 + DBL_EPSILON));
    printf(" %d\n", printf("[%a|%a]", 0.0, -0.0));
    printf(" %d\n", printf("[%#a|%#a]", 1.0, 0.0));
    printf(" %d\n", printf("[%La]", 1.0L + LDBL_EPSILON));
    return 0;
}
