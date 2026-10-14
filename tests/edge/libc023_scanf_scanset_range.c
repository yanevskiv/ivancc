// (Test) Status: 0
// (Test) Output:
// | 1 [abc]
// | 1 [xyz]
// | 1 [0123456789]
// | 1 [abcde]
// | 1 [ca-]
// | 1 [aa]
// | 1 [%+-]
// | 1 []^_`a]
// Non-standard: scanf should read a - between two in order as a range.
// Liberty: [C99:7.19.6.2p12] such a - in a scanlist is implementation-defined.

#include <stdio.h>

int main(void)
{
    char got[16];

    printf("%d [%s]\n", sscanf("abcd-", "%[a-c]", got), got);
    printf("%d [%s]\n", sscanf("xyzb", "%[^a-c]", got), got);
    printf("%d [%s]\n", sscanf("0123456789a", "%[0-9]", got), got);
    printf("%d [%s]\n", sscanf("abcdef", "%[a-c-e]", got), got);
    printf("%d [%s]\n", sscanf("ca-b", "%[c-a]", got), got);
    printf("%d [%s]\n", sscanf("aa-b", "%[a-a]", got), got);
    printf("%d [%s]\n", sscanf("%+-.", "%[%--]", got), got);
    printf("%d [%s]\n", sscanf("]^_`a", "%[]-a]", got), got);
    return 0;
}
