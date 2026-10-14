// (Test) Status: 0
// (Test) Output:
// | [%*k|5] 7
// | [%.*k|5] 8
// | [%*.*k|5|6] 11
// | [%*Ld|5] 8
// Non-standard: printf should take no argument for an invalid directive's *.
// Liberty: [C99:7.19.6.1p9] an invalid conversion is undefined behaviour.

#include <stdio.h>

int main(void)
{
    printf(" %d\n", printf("[%*k|%d]", 5, 7));
    printf(" %d\n", printf("[%.*k|%d]", 5, 7));
    printf(" %d\n", printf("[%*.*k|%d|%d]", 5, 6, 7));
    printf(" %d\n", printf("[%*Ld|%d]", 5, 7));
    return 0;
}
