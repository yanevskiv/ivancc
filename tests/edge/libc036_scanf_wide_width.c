// (Test) Status: 0
// (Test) Output:
// | 1 e9 23 2 [e]
// | 1 e9 20ac 5 [z]
// | 1 68 e9 0 3 [l]
// | 1 68 e9 0 3 [l]
// | 1 20ac 0 23 3 [e]
// Standard: scanf should count a wide conversion's width in bytes.
// Unlike: footnote 252 has the field's extent set byte by
// byte, and glibc counts wide characters, so no libc test holds this.
// [C99:7.19.6.2p12]: scanf's conversion specifiers

#include <locale.h>
#include <stddef.h>
#include <stdio.h>

#define FILL 0x23

int main(void)
{
    int n;
    int count;
    char ch = '#';
    wchar_t wide[4];

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    wide[0] = wide[1] = wide[2] = FILL;
    count = sscanf("\xc3\xa9" "e", "%2lc%n%c", wide, &n, &ch);
    printf("%d %x %x %d [%c]\n", count - 1, (unsigned) wide[0], (unsigned) wide[1], n, ch);
    count = sscanf("\xc3\xa9\xe2\x82\xac" "z", "%5lc%n%c", wide, &n, &ch);
    printf("%d %x %x %d [%c]\n", count - 1, (unsigned) wide[0], (unsigned) wide[1], n, ch);
    count = sscanf("h\xc3\xa9llo", "%3ls%n%c", wide, &n, &ch);
    printf("%d %x %x %x %d [%c]\n", count - 1, (unsigned) wide[0], (unsigned) wide[1], (unsigned) wide[2], n, ch);
    count = sscanf("h\xc3\xa9llo", "%3l[^z]%n%c", wide, &n, &ch);
    printf("%d %x %x %x %d [%c]\n", count - 1, (unsigned) wide[0], (unsigned) wide[1], (unsigned) wide[2], n, ch);
    wide[0] = wide[1] = wide[2] = FILL;
    count = sscanf("\xe2\x82\xac" "euro", "%3ls%n%c", wide, &n, &ch);
    printf("%d %x %x %x %d [%c]\n", count - 1, (unsigned) wide[0], (unsigned) wide[1], (unsigned) wide[2], n, ch);
    return 0;
}
