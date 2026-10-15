// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | log10 thousand 1 32
// | log2 eight 1 0
// | exp2 least 1 48
// | expm1 one 1
// | expm1l minus 1
// | log1p least 0
// Non-standard: log10 of a power of 10 and log2 of a power of 2 should be
// exact in a directed mode, the second raising nothing, expm1 of 1 should
// be correctly rounded, and expm1l of -1000 should raise inexact, where
// glibc's log10 and log2 miss them and raise inexact, its expm1 is an ulp
// off and its expm1l raises no inexact; exp2 should raise underflow of an
// exact subnormal, and log1p of the least subnormal should give it
// downward, as glibc's do.
// Liberty: their accuracy is implementation-defined, and so is whether they
// honour the direction; inexact and an undeserved underflow are unspecified.
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p8]:       inexact of a library function, unspecified
// [C99:F.9p9]:       an undeserved underflow, unspecified
// [C99:F.9p10]:      the current rounding direction, implementation-defined

#include <fenv.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Operands the compiler cannot fold.
static volatile double thousand = 1000.0;
static volatile double eight = 8.0;
static volatile double power = -1074.0;
static volatile double one = 1.0;
static volatile long double minus = -1000.0L;
static volatile double least = 0x1p-1074;

int main(void)
{
    double value;

    fesetround(FE_DOWNWARD);
    feclearexcept(FE_ALL_EXCEPT);
    value = log10(thousand);
    printf("log10 thousand %d %d\n", value == 3.0, fetestexcept(FE_ALL_EXCEPT));
    fesetround(FE_UPWARD);
    feclearexcept(FE_ALL_EXCEPT);
    value = log2(eight);
    printf("log2 eight %d %d\n", value == 3.0, fetestexcept(FE_ALL_EXCEPT));
    fesetround(FE_TONEAREST);
    feclearexcept(FE_ALL_EXCEPT);
    value = exp2(power);
    printf("exp2 least %d %d\n", value == 0x1p-1074, fetestexcept(FE_ALL_EXCEPT));
    printf("expm1 one %d\n", expm1(one) == 0x1.b7e151628aed3p+0);
    feclearexcept(FE_ALL_EXCEPT);
    expm1l(minus);
    printf("expm1l minus %d\n", fetestexcept(FE_INEXACT) != 0);
    fesetround(FE_DOWNWARD);
    value = log1p(least);
    fesetround(FE_TONEAREST);
    printf("log1p least %d\n", value == 0.0);
    return 0;
}
