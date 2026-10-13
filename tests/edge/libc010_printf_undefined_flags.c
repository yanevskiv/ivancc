// (Test) Status: 0
// (Test) Output:
// | [42|42|42] 10
// | [x|str] 7
// | [    x|  str] 13
// | [x    |str  ] 13
// | [x|x] 5
// Non-standard: printf should ignore a flag or precision its conversion lacks.
// Liberty: [C99 S7.19.6.1p4, p6] # on %d %u %c %s, 0 and .3 on %c are undefined.

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
