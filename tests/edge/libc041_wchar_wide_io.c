// (Test) Status: 0
// (Test) Output:
// | cut ffffffff 84 1
// | bad ffffffff 84 41 62
// | put ffffffff 84 0
// | back 20ac ffffffff 20ac 62
// | bytes 1 4321
// Non-standard: a character cut short by a stream's end should be an error
// as one that is no character is, and reading go on at the byte that was no
// part of it; a wide character with no bytes should fail when it is put,
// not when the stream is flushed; a stream should take back one wide
// character of any length, and four bytes.
// Liberty: (S7.24.3.1p3) what a stream holds after an encoding error is
// not said, and (S7.19.7.11p3) one character of pushback alone is sure.

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

#define NAME "wide.tmp"

// Make a file of n bytes, and open it to read.
static FILE *holding(const char *bytes, size_t n)
{
    FILE *file = fopen(NAME, "wb");

    fwrite(bytes, 1, n, file);
    fclose(file);
    return fopen(NAME, "rb");
}

int main(void)
{
    int i;
    FILE *file;
    wint_t wc;
    wint_t next;

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    file = holding("a\xE2\x82", 3);
    fgetwc(file);
    errno = 0;
    wc = fgetwc(file);
    printf("cut %x %d %d\n", (unsigned int) wc, errno, ferror(file) != 0);
    fclose(file);

    file = holding("a\xE2" "Ab", 4);
    fgetwc(file);
    errno = 0;
    wc = fgetwc(file);
    printf("bad %x %d", (unsigned int) wc, errno);
    wc = fgetwc(file);
    next = fgetwc(file);
    printf(" %x %x\n", (unsigned int) wc, (unsigned int) next);
    fclose(file);

    file = fopen(NAME, "wb");
    errno = 0;
    wc = fputwc(0xD800, file);
    printf("put %x %d %d\n", (unsigned int) wc, errno, ferror(file) != 0);
    fclose(file);

    file = holding("ab", 2);
    fgetwc(file);
    wc = ungetwc(0x20AC, file);
    next = ungetwc(0x20AD, file);
    printf("back %x %x", (unsigned int) wc, (unsigned int) next);
    wc = fgetwc(file);
    next = fgetwc(file);
    printf(" %x %x\n", (unsigned int) wc, (unsigned int) next);
    fclose(file);

    file = holding("ab", 2);
    fgetc(file);
    printf("bytes");
    for (i = 1; i <= 4; i++) {
        ungetc('0' + i, file);
    }
    printf(" %d ", ungetc('5', file) == EOF);
    for (i = 0; i < 4; i++) {
        putchar(fgetc(file));
    }
    putchar('\n');
    fclose(file);
    remove(NAME);
    return 0;
}
