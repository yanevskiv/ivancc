// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | pow square 1 0
// | pow root 1 0
// | pow cube 1 0
// | pow least 1 0
// | pow thirteen 1 0
// | pow overflow 1 1
// Non-standard: an exact power should be exact in every direction and
// raise nothing, and an overflow rounded to the largest finite value
// should raise overflow, where glibc's pow is an ulp off an exact power in
// the directed modes and raises inexact of it, and toward zero raises only
// inexact of 2 to 1024.
// Liberty: their accuracy is implementation-defined, and so is whether they
// honour the direction; inexact of a library function is unspecified.
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p8]:       inexact of a library function, unspecified
// [C99:F.9p10]:      the current rounding direction, implementation-defined

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Operands the compiler cannot fold.
static volatile double three = 3.0;
static volatile double nine = 9.0;
static volatile double two = 2.0;
static volatile double thirteen = 13.0;

// Print whether pow of x and y in a direction is want, and whether it
// raised any of flags.
static void show(const char *name, int mode, double x, double y, double want, int flags)
{
    int raised;
    double value;

    fesetround(mode);
    feclearexcept(FE_ALL_EXCEPT);
    value = pow(x, y);
    raised = fetestexcept(flags);
    fesetround(FE_TONEAREST);
    printf("pow %s %d %d\n", name, value == want, raised != 0);
}

int main(void)
{
    show("square", FE_DOWNWARD, three, 2.0, 9.0, FE_ALL_EXCEPT);
    show("root", FE_UPWARD, nine, 0.5, 3.0, FE_ALL_EXCEPT);
    show("cube", FE_TOWARDZERO, -three, 3.0, -27.0, FE_ALL_EXCEPT);
    show("least", FE_UPWARD, two, -1074.0, 0x1p-1074, FE_ALL_EXCEPT);
    show("thirteen", FE_DOWNWARD, thirteen, 11.0, 0x1.a145156b35p+40, FE_ALL_EXCEPT);
    show("overflow", FE_TOWARDZERO, two, 1024.0, DBL_MAX, FE_OVERFLOW);
    return 0;
}
