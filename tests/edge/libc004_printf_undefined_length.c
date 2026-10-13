// (Test) Status: 0
// (Test) Output:
// | [%Ld|%Li|%Lu|%Lo|%Lx|%LX|7] 27
// | [%lp|%hp|%Ln|7] 15
// | [%hc|%hs|%Lc|%Ls|7] 19
// | [%llc|%lls|%hhc|%js|%zc|%ts|7] 30
// | [%hf|%hhe|%llf|%jg|%zf|%tE|7] 29
// | [%l%|7] 7
// Non-standard: printf should write a length its conversion has not as text.
// Liberty: (S7.19.6.1p7) such a length modifier is undefined behaviour.

#include <stdio.h>

int main(void)
{
    printf(" %d\n", printf("[%Ld|%Li|%Lu|%Lo|%Lx|%LX|%d]", 7));
    printf(" %d\n", printf("[%lp|%hp|%Ln|%d]", 7));
    printf(" %d\n", printf("[%hc|%hs|%Lc|%Ls|%d]", 7));
    printf(" %d\n", printf("[%llc|%lls|%hhc|%js|%zc|%ts|%d]", 7));
    printf(" %d\n", printf("[%hf|%hhe|%llf|%jg|%zf|%tE|%d]", 7));
    printf(" %d\n", printf("[%l%|%d]", 7));
    return 0;
}
