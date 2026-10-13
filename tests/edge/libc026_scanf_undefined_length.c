// (Test) Status: 0
// (Test) Output:
// | 1 [a] [#]
// | 1 [c] [#]
// | 1 [e] [#]
// | 1 [g] [#]
// | 1 [i] [#]
// | 1 [k] [#]
// | 1 [m]
// | -7
// Non-standard: scanf should stop at a length its conversion has not.
// Liberty: [C99 S7.19.6.2p11] such a length modifier is undefined behaviour.

#include <stdio.h>

int main(void)
{
    int n = -7;
    char first[16] = "#";
    char second[16] = "#";

    printf("%d [%s] [%s]\n", sscanf("a b", "%s %hs", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("c d", "%s %hhc", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("e f", "%s %L[f]", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("g h", "%s %js", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("i j", "%s %zc", first, second), first, second);
    printf("%d [%s] [%s]\n", sscanf("k l", "%s %t[l]", first, second), first, second);
    printf("%d [%s]\n", sscanf("m n", "%s %Ln", first, &n), first);
    printf("%d\n", n);
    return 0;
}
