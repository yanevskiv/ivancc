// (Test) Status: 0
// (Test) Output:
// | 1 1215752191
// | 1 -1
// | 1 0
// | 1 44
// | 1 4294967295
// | 1 2345
// | 1 18446744073709551615
// | 1 9223372036854775807
// Non-standard: scanf should clamp an integer as strtol and store its low bits.
// Liberty: [C99:7.19.6.2p10] a result its object cannot hold is undefined.

#include <stdio.h>

int main(void)
{
    int count;
    int value = -7;
    long wide = -7;
    unsigned int bits = 7;
    unsigned long most = 7;
    signed char small = -7;
    unsigned short half = 7;

    count = sscanf("99999999999", "%d", &value);
    printf("%d %d\n", count, value);
    count = sscanf("9223372036854775808", "%d", &value);
    printf("%d %d\n", count, value);
    count = sscanf("-9223372036854775809", "%i", &value);
    printf("%d %d\n", count, value);
    count = sscanf("300", "%hhd", &small);
    printf("%d %d\n", count, small);
    count = sscanf("-1", "%u", &bits);
    printf("%d %u\n", count, bits);
    count = sscanf("12345", "%hx", &half);
    printf("%d %x\n", count, (unsigned int) half);
    count = sscanf("18446744073709551616", "%lu", &most);
    printf("%d %lu\n", count, most);
    count = sscanf("9999999999999999999999999999999999999999", "%ld", &wide);
    printf("%d %ld\n", count, wide);
    return 0;
}
