// (Test) Status: 0
// (Test) Output:
// | [x|str] 7
// | [    x|  str] 13
// | [x    |str  ] 13
// | [x|x] 5
// Non-standard: printf should ignore a flag or precision %lc and %ls lack.
// Liberty: # and 0 on %lc %ls, and .3 on %lc are undefined.
// [C99:7.19.6.1p4]: a conversion specification: flags, width, precision, length and specifier
// [C99:7.19.6.1p6]: printf's flags: -, +, space, # and 0

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    printf(" %d\n", printf("[%#lc|%#ls]", L'x', L"str"));
    printf(" %d\n", printf("[%05lc|%05ls]", L'x', L"str"));
    printf(" %d\n", printf("[%-05lc|%-05ls]", L'x', L"str"));
    printf(" %d\n", printf("[%.3lc|%.0lc]", L'x', L'x'));
    return 0;
}
