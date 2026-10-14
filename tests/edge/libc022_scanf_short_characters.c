// (Test) Status: 0
// (Test) Output:
// | -1
// | -1
// | -1
// | 1
// | -1
// | -7
// | 1
// | [abcde##]
// Standard: scanf should fail a %c of a width the input ends inside.
// Unlike: glibc stores what it has, so no libc test holds this.
// [C99:7.19.6.2p12]: scanf's conversion specifiers

#include <stdio.h>

int main(void)
{
    char ch = '#';
    int n = -7;
    char got[8] = "#######";

    printf("%d\n", sscanf("abc", "%5c", got));
    printf("%d\n", sscanf("abc", "%4c", got));
    printf("%d\n", sscanf("abc", "%*5c"));
    printf("%d\n", sscanf("abc", "%c%5c", &ch, got));
    printf("%d\n", sscanf("abc", "%*5c%n", &n));
    printf("%d\n", n);
    printf("%d\n", sscanf("abcde", "%5c", got));
    printf("[%s]\n", got);
    return 0;
}
