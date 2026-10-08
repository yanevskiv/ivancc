// (Test) Status: 0
// <ctype.h> declares the character classification and case mapping functions, 7.4.

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define DIGIT  "0123456789"
#define XDIGIT "0123456789abcdefABCDEF"
#define SPACE  " \t\n\v\f\r"
#define BLANK  " \t"
#define PUNCT  "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

static int where(const char *set, int c)
{
    int i;

    for (i = 0; set[i] != '\0'; i++) {
        if ((unsigned char) set[i] == c) return i;
    }
    return -1;
}

static bool in(const char *set, int c)
{
    return where(set, c) >= 0;
}

static bool same(int got, bool want)
{
    return (got != 0) == want;
}

int main(void)
{
    int (*classify)(int) = isalpha;
    int (*map)(int) = toupper;
    int c;

    for (c = 0; c <= UCHAR_MAX; c++) {
        bool alpha = in(UPPER, c) or in(LOWER, c);
        bool alnum = alpha or in(DIGIT, c);
        bool graph = alnum or in(PUNCT, c);

        if (not same(isupper(c), in(UPPER, c))) return 1;
        if (not same(islower(c), in(LOWER, c))) return 2;
        if (not same(isalpha(c), alpha)) return 3;
        if (not same(isdigit(c), in(DIGIT, c))) return 4;
        if (not same(isalnum(c), alnum)) return 5;
        if (not same(isxdigit(c), in(XDIGIT, c))) return 6;
        if (not same(isspace(c), in(SPACE, c))) return 7;
        if (not same(isblank(c), in(BLANK, c))) return 8;
        if (not same(ispunct(c), in(PUNCT, c))) return 9;
        if (not same(isgraph(c), graph)) return 10;
        if (not same(isprint(c), graph or c == ' ')) return 11;
        if (not same(iscntrl(c), c < ' ' or c == 0x7F)) return 12;
        if (toupper(c) != (in(LOWER, c) ? UPPER[where(LOWER, c)] : c)) return 13;
        if (tolower(c) != (in(UPPER, c) ? LOWER[where(UPPER, c)] : c)) return 14;
    }
    if (not classify('a') or classify('1')) return 15;
    if (map('a') != 'A' or map('A') != 'A') return 16;
    if (not (isdigit)('7') or (tolower)('Q') != 'q') return 17;
    return 0;
}
