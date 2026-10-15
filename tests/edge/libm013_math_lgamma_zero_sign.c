// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | 0 0
// | 0 0
// | 0 0
// Standard: lgamma of 1 and 2 should be +0 in every direction.
// Unlike: glibc's lgammal gives -0 of 2 under FE_DOWNWARD, where its lgamma
// and lgammaf do not.
// [C99:F.9.5.3p1]: lgamma of 1 and 2, +0

#include <fenv.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Operands the compiler cannot fold.
static volatile float one = 1.0f;
static volatile float two = 2.0f;

int main(void)
{
    fesetround(FE_DOWNWARD);
    printf("%d %d\n", signbit(lgammaf(one)) != 0, signbit(lgammaf(two)) != 0);
    printf("%d %d\n", signbit(lgamma(one)) != 0, signbit(lgamma(two)) != 0);
    printf("%d %d\n", signbit(lgammal(one)) != 0, signbit(lgammal(two)) != 0);
    fesetround(FE_TONEAREST);
    return 0;
}
