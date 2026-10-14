// (Test) Status: 0
// (Test) Output:
// | [0x1234] 8
// | [0x0000001234] 14
// | [0x00001234] 12
// | [    0x00001234] 16
// | [(nil)|     (nil)|(nil)] 24
// Non-standard: %p should take 0 and a precision as %#x does, and ignore #.
// Liberty: a precision, # and 0 on %p are undefined.
// [C99:7.19.6.1p4]: a conversion specification: flags, width, precision, length and specifier
// [C99:7.19.6.1p6]: printf's flags: -, +, space, # and 0

#include <stdio.h>

int main(void)
{
    void *ptr = (void *) 0x1234;

    printf(" %d\n", printf("[%#p]", ptr));
    printf(" %d\n", printf("[%012p]", ptr));
    printf(" %d\n", printf("[%.8p]", ptr));
    printf(" %d\n", printf("[%14.8p]", ptr));
    printf(" %d\n", printf("[%#p|%010p|%.8p]", NULL, NULL, NULL));
    return 0;
}
