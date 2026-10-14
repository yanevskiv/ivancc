// (Test) Status: 0
// (Test) Output:
// | 1 [a] -7
// | 1 [b] -7
// | 1 [c] -7
// | 1 [d] -7
// | 1 [e] -7
// | 1 [f] -7
// | 1 [g] 1
// | 1 [h] 1
// Non-standard: scanf should stop at an integer's L, or a %p's length.
// Liberty: [C99:7.19.6.2p11] such a length modifier is undefined behaviour.

#include <stdio.h>

int main(void)
{
    int count;
    void *ptr = NULL;
    long long value = -7;
    char first[16] = "#";

    count = sscanf("a 1", "%s %Ld", first, &value);
    printf("%d [%s] %lld\n", count, first, value);
    count = sscanf("b 2", "%s %Li", first, &value);
    printf("%d [%s] %lld\n", count, first, value);
    count = sscanf("c 3", "%s %Lo", first, &value);
    printf("%d [%s] %lld\n", count, first, value);
    count = sscanf("d 4", "%s %Lu", first, &value);
    printf("%d [%s] %lld\n", count, first, value);
    count = sscanf("e 5", "%s %Lx", first, &value);
    printf("%d [%s] %lld\n", count, first, value);
    count = sscanf("f 6", "%s %LX", first, &value);
    printf("%d [%s] %lld\n", count, first, value);
    count = sscanf("g 7", "%s %lp", first, &ptr);
    printf("%d [%s] %d\n", count, first, ptr == NULL);
    count = sscanf("h 8", "%s %hp", first, &ptr);
    printf("%d [%s] %d\n", count, first, ptr == NULL);
    return 0;
}
