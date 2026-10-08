// (Test) Status: 0
// <float.h> gives the characteristics of float, double and long double on x86-64 Linux (S7.7).

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>

int main(void)
{
    if (FLT_RADIX != 2) return 1;
    if (FLT_ROUNDS != 1 or FLT_EVAL_METHOD != 0) return 2;
    if (FLT_MANT_DIG != 24 or DBL_MANT_DIG != 53 or LDBL_MANT_DIG != 64) return 3;
    if (FLT_DIG != 6 or DBL_DIG != 15 or LDBL_DIG != 18 or DECIMAL_DIG != 21) return 4;
    if (FLT_MIN_EXP != -125 or DBL_MIN_EXP != -1021 or LDBL_MIN_EXP != -16381) return 5;
    if (FLT_MIN_10_EXP != -37 or DBL_MIN_10_EXP != -307 or LDBL_MIN_10_EXP != -4931) return 6;
    if (FLT_MAX_EXP != 128 or DBL_MAX_EXP != 1024 or LDBL_MAX_EXP != 16384) return 7;
    if (FLT_MAX_10_EXP != 38 or DBL_MAX_10_EXP != 308 or LDBL_MAX_10_EXP != 4932) return 8;
    if (FLT_MAX != 3.40282346638528859811704183484516925e+38F) return 9;
    if (DBL_MAX != 1.79769313486231570814527423731704357e+308) return 10;
    if (LDBL_MAX != 1.18973149535723176502126385303097021e+4932L) return 11;
    if (FLT_EPSILON != 1.1920928955078125e-7F) return 12;
    if (DBL_EPSILON != 2.220446049250313e-16) return 13;
    if (LDBL_EPSILON != 1.08420217248550443400745280086994171e-19L) return 14;
    if (FLT_MIN != 1.17549435082228750796873653722224568e-38F) return 15;
    if (DBL_MIN != 2.2250738585072014e-308) return 16;
    if (LDBL_MIN != 3.36210314311209350626267781732175260e-4932L) return 17;
    if (1.0F + FLT_EPSILON == 1.0F or 1.0 + DBL_EPSILON == 1.0 or 1.0L + LDBL_EPSILON == 1.0L) return 18;
    if (sizeof(float) != 4 or sizeof(double) != 8) return 19;
    return 0;
}
