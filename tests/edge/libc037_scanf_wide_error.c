// (Test) Status: 0
// (Test) Output:
// | -1 1 23
// | -1 1 23
// | -1 1 61
// | -1 1 23
// | 1 1 [ok]
// | -1 1 23
// | -1 0 23
// Standard: scanf should fail input that is no multibyte character as an input
// failure, EOF if nothing was assigned, and set errno to EILSEQ.
// Unlike: [C99 S7.19.6.2p9] [C99 S7.19.3p14] glibc returns 0, so no libc test holds
// this.

#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>

#define FILL 0x23

int main(void)
{
    int count;
    char word[8];
    wchar_t wide[8];

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    wide[0] = wide[1] = FILL;
    errno = 0;
    count = sscanf("\xffz", "%lc", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("\xa9z", "%ls", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("a\xffz", "%l[^z]", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    wide[0] = FILL;
    errno = 0;
    count = sscanf("\xe2\x82", "%ls", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("ok \xff", "%7s %ls", word, wide);
    printf("%d %d [%s]\n", count, errno == EILSEQ, word);
    if (setlocale(LC_CTYPE, "C") == NULL) {
        return 1;
    }
    errno = 0;
    count = sscanf("\xc3\xa9", "%ls", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    errno = 0;
    count = sscanf("", "%ls", wide);
    printf("%d %d %x\n", count, errno == EILSEQ, (unsigned) wide[0]);
    return 0;
}
