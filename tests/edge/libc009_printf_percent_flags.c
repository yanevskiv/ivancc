// (Test) Status: 0
// (Test) Output:
// | [%] 3
// | [%] 3
// | [%] 3
// | [%] 3
// | [%|7] 5
// Non-standard: %% should take flags, a width and a precision, and ignore them.
// Liberty: [C99:7.19.6.1p8] all but a complete %% is undefined behaviour.

#include <stdio.h>

int main(void)
{
    printf(" %d\n", printf("[%5%]"));
    printf(" %d\n", printf("[%-5%]"));
    printf(" %d\n", printf("[%+ #05%]"));
    printf(" %d\n", printf("[%.3%]"));
    printf(" %d\n", printf("[%*%|%d]", 5, 7));
    return 0;
}
