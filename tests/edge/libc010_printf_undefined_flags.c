// (Test) Status: 0
// (Test) Output:
// | [42|42|42] 10
// | [x|str] 7
// | [    x|  str] 13
// | [x    |str  ] 13
// | [x|x] 5
// Non-standard: printf should ignore a flag or precision its conversion lacks.
// Liberty: # on %d %u %c %s, 0 and .3 on %c are undefined.
// [C99:7.19.6.1p4]: a conversion specification: flags, width, precision, length and specifier
// [C99:7.19.6.1p6]: printf's flags: -, +, space, # and 0

#include <stdio.h>

int main(void)
{
    printf(" %d\n", printf("[%#d|%#i|%#u]", 42, 42, 42U));
    printf(" %d\n", printf("[%#c|%#s]", 'x', "str"));
    printf(" %d\n", printf("[%05c|%05s]", 'x', "str"));
    printf(" %d\n", printf("[%-05c|%-05s]", 'x', "str"));
    printf(" %d\n", printf("[%.3c|%.0c]", 'x', 'x'));
    return 0;
}
