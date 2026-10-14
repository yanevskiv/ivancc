// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | overflow 28
// | underflow 30
// | both 38
// Non-standard: feraiseexcept should raise inexact with overflow and with
// underflow, since it raises each by a division whose result is inexact.
// Liberty: whether feraiseexcept raises inexact with overflow or underflow is
// implementation-defined.
// [C99:7.6.2.3p2]: feraiseexcept, inexact with overflow or underflow implementation-defined
// [C99:F.7.6p2]:   overflow or underflow raised before inexact

#include <fenv.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

int main(void)
{
    feclearexcept(FE_ALL_EXCEPT);
    feraiseexcept(FE_OVERFLOW);
    printf("overflow %x\n", fetestexcept(FE_ALL_EXCEPT));
    feclearexcept(FE_ALL_EXCEPT);
    feraiseexcept(FE_UNDERFLOW);
    printf("underflow %x\n", fetestexcept(FE_ALL_EXCEPT));
    feclearexcept(FE_ALL_EXCEPT);
    feraiseexcept(FE_OVERFLOW | FE_UNDERFLOW);
    printf("both %x\n", fetestexcept(FE_ALL_EXCEPT));
    feclearexcept(FE_ALL_EXCEPT);
    return 0;
}
