// (Test) Status: 0
// (Test) Output:
// | [%k] 4
// | [%5k] 5
// | [%-+ #05.3k] 12
// | [%lk] 5
// | [%!|%Z|%] 9
// | [%k|7] 6
// Non-standard: printf should write a directive of no conversion as text.
// Liberty: (S7.19.6.1p9) an invalid conversion is undefined behaviour.

#include <stdio.h>

int main(void)
{
    printf(" %d\n", printf("[%k]"));
    printf(" %d\n", printf("[%5k]"));
    printf(" %d\n", printf("[%-+ #05.3k]"));
    printf(" %d\n", printf("[%lk]"));
    printf(" %d\n", printf("[%!|%Z|%]"));
    printf(" %d\n", printf("[%k|%d]", 7));
    return 0;
}
