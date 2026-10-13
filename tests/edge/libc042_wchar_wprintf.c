// (Test) Status: 0
// (Test) Output:
// | high -1 84
// | cut -1 0 ab 0
// | none -1 0 78
// | put -1 84 0
// | bytes 61 62 61
// Non-standard: %c of a byte that is no character should fail as %s of one
// does, where glibc writes WEOF; a wide string too small should hold what
// fits and a null, and errno be left alone; a wide character with no bytes
// should fail when it is put in a stream, what came before it written.
// Liberty: [C99 S7.24.2.1p14] an encoding error is a failure, which %c's
// conversion as by btowc is not said to be, and [C99 S7.24.2.3p3] a string too
// small is left unsaid.

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

#define NAME "wide.tmp"
#define SIZE 8

int main(void)
{
    int ch;
    int result;
    FILE *file;
    wchar_t out[SIZE] = {L'x', L'x', L'x', L'x', L'x', L'x', L'x', L'\0'};
    wchar_t odd[] = {L'a', 0xD800, L'b', L'\0'};

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    errno = 0;
    result = swprintf(out, SIZE, L"%c", 0xE9);
    printf("high %d %d\n", result, errno);

    errno = 0;
    result = swprintf(out, 3, L"abcdef");
    printf("cut %d %d %c%c %x\n", result, errno, (char) out[0], (char) out[1], (unsigned int) out[2]);

    out[0] = L'x';
    errno = 0;
    result = swprintf(out, 0, L"abc");
    printf("none %d %d %x\n", result, errno, (unsigned int) out[0]);

    file = fopen(NAME, "wb");
    errno = 0;
    result = fwprintf(file, L"ab%ls", odd);
    printf("put %d %d %d\n", result, errno, ferror(file) != 0);
    fclose(file);

    file = fopen(NAME, "rb");
    printf("bytes");
    while ((ch = fgetc(file)) != EOF) {
        printf(" %x", (unsigned int) ch);
    }
    putchar('\n');
    fclose(file);
    remove(NAME);
    return 0;
}
