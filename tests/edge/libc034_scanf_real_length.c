// (Test) Status: 0
// (Test) Output:
// | 1 [a] -7
// | 1 [b] -7
// | 1 [c] -7
// | 1 [d] -7
// | 1 [e] -7
// | 1 [f] -7
// | 2 [g] 7
// Non-standard: scanf should stop at a floating number's h, hh, ll, j, z or t.
// Liberty: [C99:7.19.6.2p11] such a length modifier is undefined behaviour.

#include <stdio.h>

int main(void)
{
    int count;
    char word[16] = "#";
    long double value = -7;

    count = sscanf("a 7", "%s %hf", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    count = sscanf("b 7", "%s %hhe", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    count = sscanf("c 7", "%s %llg", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    count = sscanf("d 7", "%s %ja", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    count = sscanf("e 7", "%s %zF", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    count = sscanf("f 7", "%s %tG", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    count = sscanf("g 7", "%s %LE", word, &value);
    printf("%d [%s] %Lg\n", count, word, value);
    return 0;
}
