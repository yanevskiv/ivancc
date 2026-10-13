// (Test) Status: 0
// (Test) Output:
// | [0x1234] 8
// | [0xabcdef0123456789] 20
// | [(nil)] 7
// | [              0x1234] 22
// | [0x1234              ] 22
// | [+0x1234| 0x1234] 17
// | [     (nil)|(nil)     ] 23
// Non-standard: printf should write a pointer as 0x and hex, a null one (nil).
// Liberty: (S7.19.6.1p8) %p's characters are implementation-defined.

#include <stdio.h>

int main(void)
{
    void *ptr = (void *) 0x1234;
    void *high = (void *) 0xabcdef0123456789UL;

    printf(" %d\n", printf("[%p]", ptr));
    printf(" %d\n", printf("[%p]", high));
    printf(" %d\n", printf("[%p]", NULL));
    printf(" %d\n", printf("[%20p]", ptr));
    printf(" %d\n", printf("[%-20p]", ptr));
    printf(" %d\n", printf("[%+p|% p]", ptr, ptr));
    printf(" %d\n", printf("[%10p|%-10p]", NULL, NULL));
    return 0;
}
