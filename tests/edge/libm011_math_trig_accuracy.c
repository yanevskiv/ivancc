// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | cos reduced 1
// | tan reduced 1
// | cos tiny 1
// | tanh tiny 1
// Non-standard: cos and tan of the double nearest a multiple of pi/2 should
// be correctly rounded to nearest, where glibc's are 8 and 14 ulps off, and
// cos and tanh of a tiny x should round the right way in a directed mode,
// where glibc's do not.
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
    fesetround(FE_DOWNWARD);
    show("cos tiny", cos(tiny), 0x1.fffffffffffffp-1);
    fesetround(FE_UPWARD);
    show("tanh tiny", tanh(least), DBL_MIN);
    fesetround(FE_TONEAREST);
    return 0;
}
