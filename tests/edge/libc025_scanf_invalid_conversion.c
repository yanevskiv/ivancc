// (Test) Status: 0
// (Test) Output:
// | 0
// | 0
// | 1 [a] [#]
// | 1 [c] [#]
// | 1 [e] [#]
// | 1 [g] [#]
// | 1 [i] [#]
// | 1 [k] [#]
// Non-standard: scanf should stop at a directive of no conversion, EOF never.
// Liberty: [C99/S7.19.6.2p13] an invalid conversion is undefined behaviour.

#include <stdio.h>

int main(void)
{
    char first[16] = "#";
    char second[16] = "#";

    printf("%d\n", sscanf("a b", "%k", first));
    printf("%d\n", sscanf("", "%k", first));
    printf("%d [%s] [%s]\n", sscanf("a b", "%s%k%s", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("c d", "%s %0c", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("e f", "%s %[f", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("g %h", "%s %5%%s", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("i %j", "%s %*%%s", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("k l", "%s %", first, second), first, second);
    return 0;
}
