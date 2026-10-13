// (Test) Status: 0
// (Test) Output:
// | 0 7
// | 0 -7 [#]
// | 0 1
// | 0 7
// | 0 -7
// | 0 -7
// | 2 0 [x]
// | 2 0 [x]
// Standard: scanf should fail an integer that is 0x with no digit after it.
// Unlike: (S7.19.6.2p10) glibc stores it as 0, so no libc test holds this.

#include <stdio.h>

int main(void)
{
    int count;
    int n = -7;
    int value = -7;
    char ch = '#';
    void *ptr = &n;
    unsigned int bits = 7;

    count = sscanf("0x", "%x", &bits);
    printf("%d %u\n", count, bits);
    count = sscanf("0xz", "%i%c", &value, &ch);
    printf("%d %d [%c]\n", count, value, ch);
    count = sscanf("0x", "%p", &ptr);
    printf("%d %d\n", count, ptr == &n);
    count = sscanf("0x1", "%2x", &bits);
    printf("%d %u\n", count, bits);
    count = sscanf("-0X", "%i", &value);
    printf("%d %d\n", count, value);
    count = sscanf("0x", "%*x%n", &n);
    printf("%d %d\n", count, n);
    count = sscanf("0x", "%d%c", &value, &ch);
    printf("%d %d [%c]\n", count, value, ch);
    count = sscanf("0xg", "%1x%c%x", &bits, &ch, &bits);
    printf("%d %u [%c]\n", count, bits, ch);
    return 0;
}
