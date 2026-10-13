// (Test) Status: 0
// (Test) Output:
// | -1 1 23
// | -1 1 e9
// | -1 1 68
// | -1 1 e9
// | 1 0 e9 e8 0
// Non-standard: scanf should fail a wide field that ends inside a character,
// cut there by a width or by a scanset that holds one of its bytes.
// Liberty: [C99 S7.19.6.2p12] the input shall be a sequence of multibyte
// characters, so one that is not is undefined behaviour.

#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>

#define FILL 0x23

int main(void)
{
    int count;
    wchar_t wide[8];

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    wide[0] = wide[1] = wide[2] = FILL;
    errno = 0;
    count = sscanf("\xc3\xa9", "%lc", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("\xc3\xa9\xe2\x82\xac", "%3lc", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("h\xc3\xa9llo", "%2ls", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("\xc3\xa9\xc3\xa8z", "%l[\xc3\xa9]", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("\xc3\xa9\xc3\xa8z", "%l[\xc3\xa9\xa8]", wide);
    printf("%d %d %x %x %x\n", count, errno == EILSEQ, (unsigned) wide[0], (unsigned) wide[1], (unsigned) wide[2]);
    return 0;
}
