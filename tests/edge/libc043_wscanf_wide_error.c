// (Test) Status: 0
// (Test) Output:
// | -1 1
// | -1 1
// | -1 1
// | 1 1 7
// | -1 1
// | -1 0
// Standard: wscanf should fail a wide character that has no bytes as an input
// failure, EOF if nothing was assigned, and set errno to EILSEQ.
// Unlike: glibc returns 0, so no libc test holds
// this.
// [C99:7.24.2.2p4]:  input and matching failures
// [C99:7.24.2.2p16]: fwscanf's count of items assigned, or EOF

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

int main(void)
{
    int count;
    int value = 0;
    char word[16];
    static const wchar_t odd[] = {0xD800, L'z', L'\0'};
    static const wchar_t late[] = {L'7', L' ', L'a', 0xD800, L'\0'};
    static const wchar_t high[] = {0xE9, L'\0'};

    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    errno = 0;
    count = swscanf(odd, L"%c", word);
    printf("%d %d\n", count, errno == EILSEQ);
    errno = 0;
    count = swscanf(odd, L"%s", word);
    printf("%d %d\n", count, errno == EILSEQ);
    errno = 0;
    count = swscanf(odd, L"%[^z]", word);
    printf("%d %d\n", count, errno == EILSEQ);
    errno = 0;
    count = swscanf(late, L"%d%s", &value, word);
    printf("%d %d %d\n", count, errno == EILSEQ, value);
    if (setlocale(LC_CTYPE, "C") == NULL) {
        return 1;
    }
    errno = 0;
    count = swscanf(high, L"%s", word);
    printf("%d %d\n", count, errno == EILSEQ);
    errno = 0;
    count = swscanf(L"", L"%s", word);
    printf("%d %d\n", count, errno == EILSEQ);
    return 0;
}
