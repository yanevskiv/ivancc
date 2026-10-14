// (Test) Status: 0
// (Test) Output:
// | [0x1p+0|0x1.8p+0|0x1.fep+7] 27
// | [0x1.999999999999ap-4] 22
// | [0x1.fffffffffffffp+1023] 25
// | [0x8p-3|0xcp-3|0xfp+0|0x8p+1] 29
// | [0xc.ccccccccccccccdp-7] 24
// | [0xf.fffffffffffffffp+16380] 28
// Non-standard: %a should begin a double with 1, a long double with four bits.
// Liberty: [C99:7.19.6.1p8] which nonzero digit comes first is unspecified.

#include <stdio.h>
#include <float.h>

int main(void)
{
    printf(" %d\n", printf("[%a|%a|%a]", 1.0, 1.5, 255.0));
    printf(" %d\n", printf("[%a]", 0.1));
    printf(" %d\n", printf("[%a]", DBL_MAX));
    printf(" %d\n", printf("[%La|%La|%La|%La]", 1.0L, 1.5L, 15.0L, 16.0L));
    printf(" %d\n", printf("[%La]", 0.1L));
    printf(" %d\n", printf("[%La]", LDBL_MAX));
    return 0;
}
