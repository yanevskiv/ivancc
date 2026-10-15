// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | pow square 0 32
// | pow root 0 32
// | pow cube 1 32
// | pow least 1 48
// | pow thirteen 0 32
// | pow overflow 1 40
// Non-standard: an exact power in a directed mode may be an ulp off and
// raises inexact, as glibc's pow does, but -3 cubed toward zero and 2 to
// -1074 upward should be exact, where glibc's are off, 2 to -1074 raising
// underflow as glibc's does, and 2 to 1024 rounded toward zero to the
// largest finite value should raise overflow, which glibc's does not.
// Liberty: their accuracy is implementation-defined, and so is whether they
// honour the direction; inexact and an undeserved underflow are unspecified.
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p8]:       inexact of a library function, unspecified
// [C99:F.9p9]:       an undeserved underflow, unspecified
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

// Print whether pow of x and y in a direction is want, and the flags it
// raised.
static void show(const char *name, int mode, double x, double y, double want)
{
    int raised;
    double value;

    fesetround(mode);
    feclearexcept(FE_ALL_EXCEPT);
    value = pow(x, y);
    raised = fetestexcept(FE_ALL_EXCEPT);
    fesetround(FE_TONEAREST);
    printf("pow %s %d %d\n", name, value == want, raised);
}

int main(void)
{
    show("square", FE_DOWNWARD, three, 2.0, 9.0);
    show("root", FE_UPWARD, nine, 0.5, 3.0);
    show("cube", FE_TOWARDZERO, -three, 3.0, -27.0);
    show("least", FE_UPWARD, two, -1074.0, 0x1p-1074);
    show("thirteen", FE_DOWNWARD, thirteen, 11.0, 0x1.a145156b35p+40);
    show("overflow", FE_TOWARDZERO, two, 1024.0, DBL_MAX);
    return 0;
}
