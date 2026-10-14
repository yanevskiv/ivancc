// (Test) Status: 0
// (Test) Output:
// | [(null)] 8
// | [    (null)] 12
// | [(null)    ] 12
// | [(null)] 8
// | [] 2
// | [] 2
// | [        ] 10
// Non-standard: printf should write a null string as (null), or not in part.
// Liberty: [C99:7.19.6.1p8] %s shall have an array's pointer, so it is undefined.

#include <stdio.h>

int main(void)
{
    char *str = NULL;

    printf(" %d\n", printf("[%s]", str));
    printf(" %d\n", printf("[%10s]", str));
    printf(" %d\n", printf("[%-10s]", str));
    printf(" %d\n", printf("[%.6s]", str));
    printf(" %d\n", printf("[%.5s]", str));
    printf(" %d\n", printf("[%.0s]", str));
    printf(" %d\n", printf("[%8.5s]", str));
    return 0;
}
