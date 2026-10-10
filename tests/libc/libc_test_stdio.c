// (Test) Status: 0
// (Test) Output:
// | abc
// | fputs, puts
// | fwrite 0123456789
// | a line longer than the buffer it passes through
// | after the flush
// | atexit's
// (Test) Error:
// | unbuffered
// <stdio.h> declares size_t and FILE, NULL, _IOFBF, _IOLBF, _IONBF, BUFSIZ, EOF, stderr, stdin and stdout (S7.19.1),
// fflush, setbuf and setvbuf (S7.19.5), fputc, fputs, putc, putchar and puts (S7.19.7), and fwrite (S7.19.8),
// a stream's output reaching its file by the time exit has called what atexit registered (S7.20.4.3p4).

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#define SMALL 16

#ifndef NULL
#error "NULL is not defined"
#endif

#if ! defined(_IOFBF) || ! defined(_IOLBF) || ! defined(_IONBF) || _IOFBF == _IOLBF || _IOFBF == _IONBF || _IOLBF == _IONBF
#error "_IOFBF, _IOLBF and _IONBF are not distinct"
#endif

#if ! defined(BUFSIZ) || BUFSIZ < 256
#error "BUFSIZ is not defined as at least 256"
#endif

#if ! defined(EOF) || EOF >= 0
#error "EOF is not defined as negative"
#endif

#if ! defined(stderr) || ! defined(stdin) || ! defined(stdout)
#error "stderr, stdin and stdout are not macros"
#endif

static char small[SMALL];

static void last(void)
{
    puts("atexit's");
}

int main(void)
{
    static const char longer[] = "a line longer than the buffer it passes through\n";
    FILE *streams[3];
    size_t size = sizeof(FILE);
    int bad = 0;

    streams[0] = stdin;
    streams[1] = stdout;
    streams[2] = stderr;
    if (streams[0] == NULL or streams[1] == NULL or streams[2] == NULL) return 1;
    if (streams[0] == streams[1] or streams[0] == streams[2] or streams[1] == streams[2]) return 2;
    if (size == 0) return 3;
    if (sizeof(longer) <= SMALL) return 4;

    while (bad == _IOFBF or bad == _IOLBF or bad == _IONBF) {
        bad++;
    }
    if (setvbuf(stdout, NULL, bad, 0) == 0) return 5;
    if (setvbuf(stdout, small, _IOFBF, sizeof(small)) != 0) return 6;
    setbuf(stderr, NULL);
    if (atexit(last) != 0) return 7;

    if (putchar('a') != 'a') return 8;
    if (putc('b', stdout) != 'b') return 9;
    if (fputc('c' + UCHAR_MAX + 1, stdout) != 'c') return 10;
    if (fputc('\n', stdout) != '\n') return 11;
    if (fputs("fputs, ", stdout) < 0) return 12;
    if (puts("puts") < 0) return 13;
    if (fwrite("fwrite ", 1, 7, stdout) != 7) return 14;
    if (fwrite("0123456789", 5, 2, stdout) != 2) return 15;
    if (fwrite("x", 0, 3, stdout) != 0 or fwrite("x", 1, 0, stdout) != 0) return 16;
    if (putchar('\n') != '\n') return 17;
    if (fputs(longer, stdout) < 0) return 18;
    if (fflush(stdout) != 0) return 19;
    if (fputs("after the flush\n", stdout) < 0) return 20;
    if (fflush(NULL) != 0) return 21;

    if (fputs("unbuffered\n", stderr) < 0) return 22;
    if (fflush(stderr) != 0) return 23;
    return 0;
}
