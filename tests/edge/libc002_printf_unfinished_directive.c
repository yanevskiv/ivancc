// (Test) Status: 0
// (Test) Output:
// | [% 2
// | [%5 3
// | [%-+ 4
// | [%.3 4
// | [%ll 4
// Non-standard: printf should write a directive its format ends in as text.
// Liberty: an invalid conversion is undefined behaviour.
// [C99:7.19.6.1p9]: an invalid conversion specification or argument, undefined

#include <stdio.h>

int main(void)
{
    printf(" %d\n", printf("[%"));
    printf(" %d\n", printf("[%5"));
    printf(" %d\n", printf("[%-+"));
    printf(" %d\n", printf("[%.3"));
    printf(" %d\n", printf("[%ll"));
    return 0;
}
