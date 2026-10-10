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
// <stdio.h> declares size_t and FILE, NULL, _IOFBF, _IOLBF, _IONBF, BUFSIZ, EOF, FOPEN_MAX, FILENAME_MAX, stderr, stdin and stdout (S7.19.1),
// fclose, fflush, fopen, freopen, setbuf and setvbuf (S7.19.5), fgetc, fgets, fputc, fputs, getc, getchar, gets, putc, putchar, puts and ungetc (S7.19.7),
// fread and fwrite (S7.19.8), and clearerr, feof and ferror (S7.19.10),
// a stream's output reaching its file by the time exit has called what atexit registered (S7.20.4.3p4),
// its end-of-file indicator staying set until clearerr or ungetc clears it (S7.19.7.1p3, S7.19.7.11p3),
// each of fopen's fifteen modes (S7.19.5.3p3), an update stream writing after a flush or a read to the end of its file (S7.19.5.3p6),
// FOPEN_MAX streams open at once (S7.19.1p3), and freopen giving a stream another file and clearing its indicators (S7.19.5.4p3).

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
#define HOLD  64
#define PRIME 251
#define BIG   (3 * BUFSIZ + 5)
#define WIDE  (BUFSIZ + 100)
#define MANY  (FOPEN_MAX - 3)
#define KINDS 3

#define LONGER "a line longer than the buffer it passes through\n"

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

#if ! defined(FOPEN_MAX) || FOPEN_MAX < 8
#error "FOPEN_MAX is not defined as at least 8"
#endif

#if ! defined(FILENAME_MAX) || FILENAME_MAX < 1
#error "FILENAME_MAX is not defined as positive"
#endif

#if ! defined(stderr) || ! defined(stdin) || ! defined(stdout)
#error "stderr, stdin and stdout are not macros"
#endif

static char outbuf[SMALL];
static char inbuf[SMALL];
static char filebuf[SMALL];
static unsigned char wide[WIDE];

static void last(void)
{
    puts("atexit's");
}

static int put(const char *name, const char *mode, const char *str)
{
    FILE *file = fopen(name, mode);

    return file != NULL and fputs(str, file) >= 0 and fclose(file) == 0;
}

static int holds(const char *name, const char *str)
{
    FILE *file = fopen(name, "r");
    char got[HOLD];
    size_t len;

    if (file == NULL) return 0;
    len = fread(got, 1, sizeof(got) - 1, file);
    got[len] = '\0';
    return feof(file) and not ferror(file) and fclose(file) == 0 and strcmp(got, str) == 0;
}

int main(void)
{
    static const char *const modes[] = {"r", "rb", "r+", "rb+", "r+b", "a", "ab", "a+", "ab+", "a+b", "w", "wb", "w+", "wb+", "w+b"};
    static const unsigned char bytes[] = {0, UCHAR_MAX, '\n', '\r', '\n', 26, 'a', 0};
    static const int kinds[KINDS] = {_IOFBF, _IOLBF, _IONBF};
    static const char longer[] = LONGER;
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    FILE *streams[3];
    FILE *many[MANY];
    FILE *file;
    char name[] = "many?";
    char block[HOLD];
    char line[32];
    size_t size = sizeof(FILE);
    size_t k;
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

    if (fopen("missing", "r") != NULL or fopen("missing", "rb") != NULL or fopen("missing", "r+") != NULL) return 50;
    file = fopen("file", "w");
    if (file == NULL or file == stdin or file == stdout or file == stderr) return 51;
    if (fputs("abcdef\n", file) < 0 or ferror(file) or fclose(file) != 0) return 52;
    if (not holds("file", "abcdef\n")) return 53;
    if (not put("file", "a", "gh\n") or not holds("file", "abcdef\ngh\n")) return 54;
    if (not put("file", "w", "abcdef") or not holds("file", "abcdef")) return 55;
    if (not put("made by a", "a", "a") or not put("made by w+", "w+", "w+") or not put("made by a+", "a+", "a+")) return 56;
    if (not holds("made by a", "a") or not holds("made by w+", "w+") or not holds("made by a+", "a+")) return 57;

    file = fopen("file", "r+");
    if (file == NULL or fputs("XY", file) < 0 or fflush(file) != 0) return 58;
    if (fgetc(file) != 'c' or fread(block, 1, 8, file) != 3 or memcmp(block, "def", 3) != 0) return 59;
    if (not feof(file) or fputs("gh", file) < 0 or fclose(file) != 0) return 60;
    if (not holds("file", "XYcdefgh")) return 61;
    file = fopen("file", "w+");
    if (file == NULL or fputs("hello", file) < 0 or fflush(file) != 0) return 62;
    if (fgetc(file) != EOF or not feof(file) or fclose(file) != 0) return 63;
    file = fopen("file", "a+");
    if (file == NULL or fputs(", world", file) < 0 or fflush(file) != 0) return 64;
    if (fgetc(file) != EOF or fclose(file) != 0 or not holds("file", "hello, world")) return 65;
    for (k = 0; k < sizeof(modes) / sizeof(modes[0]); k++) {
        file = fopen("file", modes[k]);
        if (file == NULL or fclose(file) != 0) return 66;
    }
    if (not holds("file", "")) return 67;

    file = fopen("bytes", "wb");
    if (file == NULL or fwrite(bytes, 1, sizeof(bytes), file) != sizeof(bytes) or fclose(file) != 0) return 68;
    file = fopen("bytes", "rb");
    if (file == NULL or fread(block, 1, sizeof(block), file) != sizeof(bytes) or memcmp(block, bytes, sizeof(bytes)) != 0) return 69;
    if (not feof(file) or fclose(file) != 0) return 70;
    file = fopen("big", "wb");
    if (file == NULL) return 71;
    for (k = 0; k < BIG; k++) {
        if (fputc((int) (k % PRIME), file) == EOF) return 72;
    }
    if (fclose(file) != 0) return 73;
    file = fopen("big", "rb");
    if (file == NULL) return 74;
    for (k = 0; k < BIG; k++) {
        if (fgetc(file) != (int) (k % PRIME)) return 75;
    }
    if (fgetc(file) != EOF or not feof(file) or ferror(file) or fclose(file) != 0) return 76;
    for (k = 0; k < sizeof(wide); k++) {
        wide[k] = (unsigned char) (k % PRIME);
    }
    file = fopen("big", "wb");
    if (file == NULL or fputc('!', file) != '!' or fwrite(wide, sizeof(wide), 1, file) != 1 or fclose(file) != 0) return 77;
    memset(wide, 0, sizeof(wide));
    file = fopen("big", "rb");
    if (file == NULL or fgetc(file) != '!' or fread(wide, 1, sizeof(wide), file) != sizeof(wide)) return 78;
    for (k = 0; k < sizeof(wide); k++) {
        if (wide[k] != k % PRIME) return 79;
    }
    if (fgetc(file) != EOF or fclose(file) != 0) return 80;

    for (k = 0; k < MANY; k++) {
        name[strlen(name) - 1] = digits[k];
        many[k] = fopen(name, "w");
        if (many[k] == NULL or fputc(digits[k], many[k]) == EOF) return 81;
    }
    for (k = 1; k < MANY; k += 2) {
        if (fclose(many[k]) != 0) return 82;
    }
    for (k = 0; k < MANY; k += 2) {
        if (fclose(many[k]) != 0) return 83;
    }
    for (k = 0; k < MANY; k++) {
        name[strlen(name) - 1] = digits[k];
        line[0] = digits[k];
        line[1] = '\0';
        if (not holds(name, line)) return 84;
    }

    for (k = 0; k < KINDS; k++) {
        file = fopen("file", "w");
        if (file == NULL or setvbuf(file, kinds[k] == _IONBF ? NULL : filebuf, kinds[k], sizeof(filebuf)) != 0) return 85;
        if (fputs(longer, file) < 0 or fputs("two\n", file) < 0 or fclose(file) != 0) return 86;
        file = fopen("file", "r");
        if (file == NULL or setvbuf(file, kinds[k] == _IONBF ? NULL : filebuf, kinds[k], sizeof(filebuf)) != 0) return 87;
        if (fgets(block, sizeof(block), file) != block or strcmp(block, longer) != 0) return 88;
        if (fgets(block, sizeof(block), file) != block or strcmp(block, "two\n") != 0) return 89;
        if (fgets(block, sizeof(block), file) != NULL or not feof(file) or fclose(file) != 0) return 90;
    }

    file = fopen("file", "w");
    if (file == NULL or fputs("first", file) < 0) return 91;
    if (freopen("other", "w", file) != file or fputs("second", file) < 0) return 92;
    if (freopen("file", "r", file) != file or fgets(line, sizeof(line), file) != line or strcmp(line, "first") != 0) return 93;
    if (not feof(file) or freopen("other", "a+", file) != file or feof(file) or ferror(file)) return 94;
    if (fputs("!", file) < 0 or fclose(file) != 0 or not holds("other", "second!")) return 95;
    file = fopen("file", "r");
    if (file == NULL or fgetc(file) != 'f' or ungetc('?', file) != '?') return 96;
    if (freopen("other", "r", file) != file or fgets(line, sizeof(line), file) != line or strcmp(line, "second!") != 0) return 97;
    if (freopen("missing", "r", file) != NULL) return 98;
    if (not feof(stdin) or freopen("other", "r", stdin) != stdin or feof(stdin) or ferror(stdin)) return 99;
    if (getchar() != 's' or gets(line) != line or strcmp(line, "econd!") != 0) return 100;
    if (fclose(stdin) != 0) return 101;
    return 0;
}
