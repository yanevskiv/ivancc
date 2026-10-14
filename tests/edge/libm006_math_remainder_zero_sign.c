// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | 0 1
// | 0 1
// | 0 1
// Standard: a zero remainder should take x's sign in every direction.
// Unlike: glibc's remainderf gives -0 of a positive x and +0 of a negative
// one under FE_DOWNWARD, where its remainder and remainderl do not.
// [C99:7.12.10.2p2]: remainder, its footnote 210 giving a zero x's sign
// [C99:F.9.7.2p1]:   remainder, as IEC 60559's

#include <fenv.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Operands the compiler cannot fold.
static volatile float big = 0x1.7899f8p+59f;
static volatile float step = 0x1p+38f;

int main(void)
{
    fesetround(FE_DOWNWARD);
    printf("%d %d\n", signbit(remainderf(big, step)) != 0, signbit(remainderf(-big, step)) != 0);
    printf("%d %d\n", signbit(remainder(big, step)) != 0, signbit(remainder(-big, step)) != 0);
    printf("%d %d\n", signbit(remainderl(big, step)) != 0, signbit(remainderl(-big, step)) != 0);
    fesetround(FE_TONEAREST);
    return 0;
}
