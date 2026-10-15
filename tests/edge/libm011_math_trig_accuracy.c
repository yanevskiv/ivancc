// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | cos reduced 1
// | tan reduced 1
// | sinhl nearest 1
// | cos tiny 1
// | tanh tiny 1
// Non-standard: the trigonometric and hyperbolic functions should be within
// 2 ulps in every direction, where glibc's cos and tan of the double
// nearest a multiple of pi/2 are 8 and 14 ulps off to nearest, its sinhl
// over 2 ulps off to nearest, and its cos and tanh of a tiny x rounded the
// wrong way in a directed mode.
// Liberty: their accuracy is implementation-defined, and so is whether they
// honour the direction.
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p10]:      the current rounding direction, implementation-defined

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Operands the compiler cannot fold.
static volatile double reduced = 0x1.6ac5b262ca1ffp+849;
static volatile long double hyperbolic = 0xf.bf9f3d87bff0cap-10L;
static volatile double tiny = 0x1p-30;
static volatile double least = DBL_MIN;

// Print whether a value is want.
static void show(const char *name, long double value, long double want)
{
    printf("%s %d\n", name, value == want);
}

int main(void)
{
    show("cos reduced", cos(reduced), -0x1.14ae72e6ba22fp-61);
    show("tan reduced", tan(reduced), -0x1.d9ba9a7975636p+60);
    show("sinhl nearest", sinhl(hyperbolic), 0xf.bfc7ed594726667p-10L);
    fesetround(FE_DOWNWARD);
    show("cos tiny", cos(tiny), 0x1.fffffffffffffp-1);
    fesetround(FE_UPWARD);
    show("tanh tiny", tanh(least), DBL_MIN);
    fesetround(FE_TONEAREST);
    return 0;
}
