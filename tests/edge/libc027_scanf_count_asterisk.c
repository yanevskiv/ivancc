// (Test) Status: 0
// (Test) Output:
// | 1 [a] [#]
// | 1 [c] [#]
// | -7
// | 1 [e] [#]
// | 1 [g]
// | 1 -7
// | 0
// Non-standard: scanf should stop at a %n with an asterisk or a width.
// Liberty: [C99 S7.19.6.2p12] either with %n is undefined behaviour.

#include <stdio.h>

int main(void)
{
    int n = -7;
    int m = -7;
    char first[16] = "#";
    char second[16] = "#";

    printf("%d [%s] [%s]\n", sscanf("a b", "%s%*n%s", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("c d", "%s%5n%s", first, &n, second), first, second);
    printf("%d\n", n);
    printf("%d [%s] [%s]\n", sscanf("e f", "%s%*5n%s", first, second), first, second);
    printf("%d [%s]\n", sscanf("g h", "%s%n%*n%n", first, &n, &m), first);
    printf("%d %d\n", n, m);
    printf("%d\n", sscanf("", "%*n"));
    return 0;
}
