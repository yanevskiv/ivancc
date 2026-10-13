// (Test) Status: 0
// (Test) Output:
// | 1 1
// | 1 1f
// | 1 2f
// | 1 ffffffffffffffff
// | 2 1 [x]
// | 0 1
// | 0 1
// | 1 123
// Non-standard: scanf's %p should read (nil), or hex with a sign or 0x or not.
// Liberty: (S7.19.6.2p12) the sequences %p matches are implementation-defined.

#include <stdio.h>

int main(void)
{
    int count;
    char ch = '#';
    void *ptr = &ch;

    count = sscanf("(nil)", "%p", &ptr);
    printf("%d %d\n", count, ptr == NULL);
    count = sscanf("1f", "%p", &ptr);
    printf("%d %lx\n", count, (unsigned long) ptr);
    count = sscanf("0X2F", "%p", &ptr);
    printf("%d %lx\n", count, (unsigned long) ptr);
    count = sscanf("-1", "%p", &ptr);
    printf("%d %lx\n", count, (unsigned long) ptr);
    count = sscanf(" (NiL)x", "%p%c", &ptr, &ch);
    printf("%d %d [%c]\n", count, ptr == NULL, ch);
    count = sscanf("(nil", "%p", &ptr);
    printf("%d %d\n", count, ptr == NULL);
    count = sscanf("(nil)", "%4p", &ptr);
    printf("%d %d\n", count, ptr == NULL);
    count = sscanf("0x12345", "%5p", &ptr);
    printf("%d %lx\n", count, (unsigned long) ptr);
    return 0;
}
