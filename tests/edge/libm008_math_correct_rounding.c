// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | cbrt cube 1 0
// | cbrt eighth 1 0
// | cbrt nearest 1
// | cbrt subnormal 1
// | hypot nearest 1
// | hypotl nearest 1
// | hypot subnormal 1 0
// Non-standard: cbrt and hypot should be correctly rounded in the current
// direction, an exact result raising nothing, where glibc's are an ulp off
// for some, its cbrt raises inexact of a cube, and its hypot underflow of
// an exact subnormal.
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
static volatile double cube = -27.0;
static volatile double eighth = 0.125;
static volatile double two = 2.0;
static volatile double least = 0x1p-1074;
static volatile double side = 0x1.00000148a4a69p+61;
static volatile double other = 0x1.0c0001580c5e6p+59;
static volatile long double sidel = 0xd.6ac296ed96160d1p-3039L;
static volatile long double otherl = 0xa.2ae776600bbcb5ep-3041L;
static volatile double three = 0x1.8p-1073;
static volatile double four = 0x1p-1072;

int main(void)
{
    double root;

    feclearexcept(FE_ALL_EXCEPT);
    root = cbrt(cube);
    printf("cbrt cube %d %d\n", root == -3.0, fetestexcept(FE_ALL_EXCEPT));
    root = cbrt(eighth);
    printf("cbrt eighth %d %d\n", root == 0.5, fetestexcept(FE_ALL_EXCEPT));
    printf("cbrt nearest %d\n", cbrt(two) == 0x1.428a2f98d728bp+0);
    printf("cbrt subnormal %d\n", cbrt(least) == 0x1p-358);
    printf("hypot nearest %d\n", hypot(side, other) == 0x1.089f5520b45c6p+61);
    printf("hypotl nearest %d\n", hypotl(sidel, otherl) == 0xd.a7db95ad825e114p-3039L);
    feclearexcept(FE_ALL_EXCEPT);
    root = hypot(three, four);
    printf("hypot subnormal %d %d\n", root == 0x1.4p-1072, fetestexcept(FE_ALL_EXCEPT));
    return 0;
}
