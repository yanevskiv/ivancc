// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | lgamma zero 1
// | tgamma nearest 1
// | erfc downward 1
// | tgamma exact 1
// Non-standard: lgamma near a zero of it and tgamma near -9 should be
// correctly rounded to nearest, where glibc's are 2.85 and 4.44 ulps off,
// erfc should round the right way downward, where glibc's is 4.44 ulps off,
// and tgamma of 3 should be exact, where glibc's raises inexact.
// Liberty: their accuracy is implementation-defined, and so is whether they
// honour the direction; whether they raise inexact is unspecified.
// [C99:5.2.4.2.2p5]: the accuracy of <math.h>, implementation-defined
// [C99:F.9p8]:       inexact of a library function, unspecified
// [C99:F.9p10]:      the current rounding direction, implementation-defined

#include <fenv.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Operands the compiler cannot fold.
static volatile double near_zero = -0xa.04352bf85b6ep-1;
static volatile double near_nine = -0x9.00002e3bb47e8p+0;
static volatile double tail = 0xb.530dbd2f697a8p-3;
static volatile double three = 3.0;

// Print whether a value is want.
static void show(const char *name, long double value, long double want)
{
    printf("%s %d\n", name, value == want);
}

int main(void)
{
    double exact;

    show("lgamma zero", lgamma(near_zero), -0xb.5fe23e0fe72cp-45);
    show("tgamma nearest", tgamma(near_nine), 0xf.ffffffa9c5a4p-4);
    fesetround(FE_DOWNWARD);
    show("erfc downward", erfc(tail), 0xb.988527d1a8fc8p-8);
    fesetround(FE_TONEAREST);
    feclearexcept(FE_ALL_EXCEPT);
    exact = tgamma(three);
    show("tgamma exact", exact == 2.0 && ! fetestexcept(FE_INEXACT), 1);
    return 0;
}
