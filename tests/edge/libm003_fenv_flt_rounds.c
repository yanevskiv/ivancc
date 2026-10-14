// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | 1 1
// | 2 2
// | 3 3
// | 0 0
// | 1 1
// Standard: FLT_ROUNDS should follow each direction fesetround sets.
// Unlike: gcc's <float.h> keeps it 1 with glibc, so no libc test holds this.
// [C99:5.2.4.2.2p7]: FLT_ROUNDS, its footnote 18 following fesetround
// [C99:F.3p1]:       FLT_ROUNDS 0 to 3, the four rounding directions

#include <fenv.h>
#include <float.h>
#include <stdio.h>

#pragma STDC FENV_ACCESS ON

// Show FLT_ROUNDS, and again through a variable, under one direction.
static void show(int round)
{
    int rounds;

    fesetround(round);
    rounds = FLT_ROUNDS;
    printf("%d %d\n", FLT_ROUNDS, rounds);
}

int main(void)
{
    show(FE_TONEAREST);
    show(FE_UPWARD);
    show(FE_DOWNWARD);
    show(FE_TOWARDZERO);
    show(FE_TONEAREST);
    return 0;
}
