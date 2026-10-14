// Includable helper file for <fenv.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <fenv.h>

// The tests test flags and change modes, which C99 allows only with this on.
#pragma STDC FENV_ACCESS ON

// Defines.
#define EXCEPTS 5
#define MODES   4

// Objects.
static const int excepts[EXCEPTS] = {FE_DIVBYZERO, FE_INEXACT, FE_INVALID, FE_OVERFLOW, FE_UNDERFLOW};
static const int modes[MODES] = {FE_DOWNWARD, FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD};

// Operands the compiler cannot fold, so each operation runs when the test does.
static volatile double one = 1.0;
static volatile double three = 3.0;
static volatile long double lone = 1.0L;
static volatile long double lthree = 3.0L;

// Return 1 / 3 in double, rounded in the current direction.
static double third(void)
{
    return one / three;
}

// Return 1 / 3 in long double, rounded in the current direction.
static long double lthird(void)
{
    return lone / lthree;
}
