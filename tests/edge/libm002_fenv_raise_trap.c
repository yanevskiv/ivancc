// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | raising
// Non-standard: an exception unmasked through fenv_t's control words should
// trap in feraiseexcept itself, as the arithmetic that raises it would.
// Liberty: what fenv_t holds is the implementation's, and C99 names no way
// to unmask an exception.
// [C99:7.6p3]:        fenv_t, the entire environment
// [C99:7.6.2.3p2]:    feraiseexcept, the exceptions raised as arithmetic raises them
// [C99:7.14.1.1p3]:   a SIGFPE handler's return, undefined
// [C99:7.14.1.1p5]:   a handler's calls, _Exit among those defined

#include <fenv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#pragma STDC FENV_ACCESS ON

// Leave as the trap's proof, since returning would divide again.
static void on_fpe(int sig)
{
    (void) sig;
    _Exit(EXIT_SUCCESS);
}

int main(void)
{
    fenv_t env;

    signal(SIGFPE, on_fpe);
    feclearexcept(FE_ALL_EXCEPT);
    fegetenv(&env);
    env.fe_control &= (unsigned short) ~FE_INVALID;
    env.fe_mxcsr &= (unsigned int) ~(FE_INVALID << _FENV_SSE_MASK_SHIFT);
    fesetenv(&env);
    printf("raising\n");
    fflush(stdout);
    feraiseexcept(FE_INVALID);
    printf("not trapped\n");
    return EXIT_FAILURE;
}
