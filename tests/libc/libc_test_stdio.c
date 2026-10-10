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
// (Test) Input:
// | abc
// | a line read in two parts
// | gets takes this one
// | 0123456789abcdefghijklmnopqrstuvwxyz
// | a block over a refill
// | end
// <stdio.h> declares size_t and FILE, NULL, _IOFBF, _IOLBF, _IONBF, BUFSIZ, EOF, stderr, stdin and stdout (S7.19.1),
// fflush, setbuf and setvbuf (S7.19.5), fgetc, fgets, fputc, fputs, getc, getchar, gets, putc, putchar, puts and ungetc (S7.19.7),
// fread and fwrite (S7.19.8), and clearerr, feof and ferror (S7.19.10),
// a stream's output reaching its file by the time exit has called what atexit registered (S7.20.4.3p4),
// and its end-of-file indicator staying set until clearerr or ungetc clears it (S7.19.7.1p3, S7.19.7.11p3).

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

static char outbuf[SMALL];
static char inbuf[SMALL];

static void last(void)
{
    puts("atexit's");
}

int main(void)
{
    static const char longer[] = "a line longer than the buffer it passes through\n";
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    FILE *streams[3];
    char block[40];
    char line[32];
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
    if (setvbuf(stdout, outbuf, _IOFBF, sizeof(outbuf)) != 0) return 6;
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

    if (setvbuf(stdin, inbuf, _IOFBF, sizeof(inbuf)) != 0) return 24;
    if (feof(stdin) or ferror(stdin)) return 25;
    if (getchar() != 'a') return 26;
    if (getc(stdin) != 'b') return 27;
    if (fgetc(stdin) != 'c') return 28;
    if (ungetc(EOF, stdin) != EOF) return 29;
    if (ungetc('x' + UCHAR_MAX + 1, stdin) != 'x') return 30;
    if (fgetc(stdin) != 'x' or fgetc(stdin) != '\n') return 31;
    if (fgets(line, 8, stdin) != line or strcmp(line, "a line ") != 0) return 32;
    if (fgets(line, sizeof(line), stdin) != line or strcmp(line, "read in two parts\n") != 0) return 33;
    if (gets(line) != line or strcmp(line, "gets takes this one") != 0) return 34;
    if (fread(block, 4, 9, stdin) != 9 or memcmp(block, digits, 36) != 0) return 35;
    if (fread(block, 0, 3, stdin) != 0 or fread(block, 1, 0, stdin) != 0) return 36;
    if (fgetc(stdin) != '\n') return 37;
    if (fread(block, 7, 3, stdin) != 3 or memcmp(block, "a block over a refill", 21) != 0) return 38;
    if (ungetc(UCHAR_MAX, stdin) != UCHAR_MAX or fgetc(stdin) != UCHAR_MAX) return 39;
    if (fgetc(stdin) != '\n' or feof(stdin)) return 40;
    if (fread(block, 3, 2, stdin) != 1 or memcmp(block, "end", 3) != 0) return 41;
    if (not feof(stdin) or ferror(stdin)) return 42;
    if (fgetc(stdin) != EOF) return 43;
    if (fgets(line, sizeof(line), stdin) != NULL or strcmp(line, "gets takes this one") != 0) return 44;
    if (gets(line) != NULL) return 45;
    clearerr(stdin);
    if (feof(stdin) or ferror(stdin)) return 46;
    if (fgetc(stdin) != EOF or not feof(stdin)) return 47;
    if (ungetc('z', stdin) != 'z' or feof(stdin)) return 48;
    if (fgets(line, sizeof(line), stdin) != line or strcmp(line, "z") != 0 or not feof(stdin)) return 49;
    return 0;
}
