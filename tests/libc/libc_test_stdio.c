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
// <stdio.h> declares size_t, FILE and fpos_t, NULL, _IOFBF, _IOLBF, _IONBF, BUFSIZ, EOF, FOPEN_MAX, FILENAME_MAX, L_tmpnam, SEEK_CUR, SEEK_END, SEEK_SET, TMP_MAX, stderr, stdin and stdout (S7.19.1),
// remove, rename, tmpfile and tmpnam (S7.19.4), fclose, fflush, fopen, freopen, setbuf and setvbuf (S7.19.5), fgetc, fgets, fputc, fputs, getc, getchar, gets, putc, putchar, puts and ungetc (S7.19.7),
// fread and fwrite (S7.19.8), fgetpos, fseek, fsetpos, ftell and rewind (S7.19.9), and clearerr, feof, ferror and perror (S7.19.10),
// a stream's output reaching its file by the time exit has called what atexit registered (S7.20.4.3p4),
// its end-of-file indicator staying set until clearerr or ungetc clears it (S7.19.7.1p3, S7.19.7.11p3),
// each of fopen's fifteen modes (S7.19.5.3p3), an update stream writing after a flush or a read to the end of its file (S7.19.5.3p6),
// FOPEN_MAX streams open at once (S7.19.1p3), freopen giving a stream another file and clearing its indicators (S7.19.5.4p3),
// a binary stream's position counted in characters from its file's start (S7.19.9.4p2) and one less after ungetc (S7.19.7.11p5),
// fseek dropping a pushed-back character and the end-of-file indicator (S7.19.9.2p5), a text stream returning to what ftell gave (S7.19.9.2p4),
// an update stream reading and writing either side of a positioning call (S7.19.5.3p6), an append stream writing at its file's end wherever it is positioned (S7.19.5.3p5),
// a removed file opening no more (S7.19.4.1p2) and a renamed one by its new name alone (S7.19.4.2p2), tmpfile's streams each reading back what it was written (S7.19.4.3p2),
// tmpnam giving a different name each call, one no file has (S7.19.4.4p2), in its array or its own (S7.19.4.4p3),
// and perror writing its string, a colon and a space, where it has one, then strerror's text and a new-line to stderr (S7.19.10.4p2).

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
#define NAMES 25

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

#if ! defined(L_tmpnam) || L_tmpnam < 1
#error "L_tmpnam is not defined as positive"
#endif

#if ! defined(TMP_MAX) || TMP_MAX < 25
#error "TMP_MAX is not defined as at least 25"
#endif

#if ! defined(SEEK_CUR) || ! defined(SEEK_END) || ! defined(SEEK_SET) || SEEK_CUR == SEEK_END || SEEK_CUR == SEEK_SET || SEEK_END == SEEK_SET
#error "SEEK_CUR, SEEK_END and SEEK_SET are not distinct"
#endif

#if ! defined(stderr) || ! defined(stdin) || ! defined(stdout)
#error "stderr, stdin and stdout are not macros"
#endif

static char outbuf[SMALL];
static char inbuf[SMALL];
static char filebuf[SMALL];
static unsigned char wide[WIDE];
static char names[NAMES][L_tmpnam];

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

static int reads(FILE *file, const char *prefix, int number)
{
    char want[HOLD];
    char got[HOLD];

    strcat(strcat(strcpy(want, prefix), strerror(number)), "\n");
    return fgets(got, sizeof(got), file) == got and strcmp(got, want) == 0;
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
    FILE *other;
    char name[] = "many?";
    char block[HOLD];
    char line[32];
    size_t size = sizeof(FILE);
    size_t k;
    size_t n;
    fpos_t pos;
    long at;
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

    file = fopen("pos", "w+b");
    if (file == NULL or ftell(file) != 0 or fwrite(digits, 1, 36, file) != 36 or ftell(file) != 36) return 101;
    rewind(file);
    if (ftell(file) != 0 or fgetc(file) != '0' or ftell(file) != 1) return 102;
    if (fseek(file, 10, SEEK_SET) != 0 or fgetc(file) != 'a' or ftell(file) != 11) return 103;
    if (fseek(file, 5, SEEK_CUR) != 0 or ftell(file) != 16 or fgetc(file) != 'g') return 104;
    if (fseek(file, -7, SEEK_CUR) != 0 or ftell(file) != 10 or fgetc(file) != 'a') return 105;
    if (ungetc('?', file) != '?' or ftell(file) != 10 or fseek(file, 0, SEEK_CUR) != 0 or fgetc(file) != 'a') return 106;
    if (fread(block, 1, sizeof(block), file) != 25 or not feof(file)) return 107;
    if (fseek(file, 0, SEEK_CUR) != 0 or feof(file) or ftell(file) != 36) return 108;
    if (fgetc(file) != EOF or not feof(file)) return 109;
    rewind(file);
    if (feof(file) or ferror(file) or fgetc(file) != '0') return 110;
    if (fseek(file, -1, SEEK_SET) == 0) return 111;
    if (fseek(file, 10, SEEK_SET) != 0 or fread(block, 1, 2, file) != 2 or memcmp(block, "ab", 2) != 0) return 112;
    if (fseek(file, 0, SEEK_CUR) != 0 or fwrite("XY", 1, 2, file) != 2 or ftell(file) != 14) return 113;
    if (fseek(file, -4, SEEK_CUR) != 0 or fread(block, 1, 6, file) != 6 or memcmp(block, "abXYef", 6) != 0) return 114;
    if (fgetpos(file, &pos) != 0 or fgetc(file) != 'g' or fgetc(file) != 'h') return 115;
    if (fsetpos(file, &pos) != 0 or fgetc(file) != 'g' or ftell(file) != 17) return 116;
    if (fclose(file) != 0 or not holds("pos", "0123456789abXYefghijklmnopqrstuvwxyz")) return 117;

    if (not put("pos", "w", "one\ntwo\nthree\n")) return 118;
    file = fopen("pos", "r+");
    if (file == NULL or fgets(line, sizeof(line), file) != line or strcmp(line, "one\n") != 0) return 119;
    at = ftell(file);
    if (at < 0 or fgets(line, sizeof(line), file) != line or strcmp(line, "two\n") != 0) return 120;
    if (fgets(line, sizeof(line), file) != line or strcmp(line, "three\n") != 0) return 121;
    if (fseek(file, at, SEEK_SET) != 0 or fgets(line, sizeof(line), file) != line or strcmp(line, "two\n") != 0) return 122;
    if (fseek(file, 0, SEEK_END) != 0 or fgetc(file) != EOF or not feof(file)) return 123;
    if (fseek(file, 0, SEEK_END) != 0 or feof(file) or fputs("four\n", file) < 0) return 124;
    if (fseek(file, at, SEEK_SET) != 0 or fgets(line, sizeof(line), file) != line or strcmp(line, "two\n") != 0) return 125;
    if (fclose(file) != 0 or not holds("pos", "one\ntwo\nthree\nfour\n")) return 126;

    if (not put("pos", "w", "abc")) return 127;
    file = fopen("pos", "a");
    if (file == NULL or fseek(file, 0, SEEK_SET) != 0 or fputs("def", file) < 0 or fclose(file) != 0) return 128;
    file = fopen("pos", "a+b");
    if (file == NULL or fseek(file, 1, SEEK_SET) != 0 or fgetc(file) != 'b') return 129;
    if (fseek(file, 0, SEEK_SET) != 0 or fputc('g', file) != 'g' or fclose(file) != 0) return 130;
    if (not holds("pos", "abcdefg")) return 131;

    file = fopen("big", "rb");
    if (file == NULL or fseek(file, BUFSIZ + 50, SEEK_SET) != 0 or fgetc(file) != (BUFSIZ + 49) % PRIME) return 132;
    if (ftell(file) != BUFSIZ + 51 or fseek(file, -(long) BUFSIZ, SEEK_CUR) != 0 or fgetc(file) != 50) return 133;
    if (fseek(file, 3, SEEK_CUR) != 0 or fgetc(file) != 54 or ftell(file) != 56 or fclose(file) != 0) return 134;

    if (remove("missing") == 0 or rename("missing", "other name") == 0) return 135;
    if (not put("old", "w", "moved") or rename("old", "new") != 0) return 136;
    if (fopen("old", "r") != NULL or not holds("new", "moved")) return 137;
    if (remove("new") != 0 or fopen("new", "r") != NULL or remove("new") == 0) return 138;

    file = tmpfile();
    other = tmpfile();
    if (file == NULL or other == NULL or file == other) return 139;
    if (fwrite(bytes, 1, sizeof(bytes), file) != sizeof(bytes) or fputs("the other's", other) < 0) return 140;
    rewind(file);
    rewind(other);
    if (fread(block, 1, sizeof(block), file) != sizeof(bytes) or memcmp(block, bytes, sizeof(bytes)) != 0) return 141;
    if (fgets(line, sizeof(line), other) != line or strcmp(line, "the other's") != 0) return 142;
    if (fseek(file, 2, SEEK_SET) != 0 or fputc('!', file) != '!' or fseek(file, 2, SEEK_SET) != 0 or fgetc(file) != '!') return 143;
    if (fclose(file) != 0 or fclose(other) != 0) return 144;

    for (k = 0; k < NAMES; k++) {
        char *got = tmpnam(k % 2 == 0 ? names[k] : NULL);

        if (got == NULL or strlen(got) >= L_tmpnam or (k % 2 == 0 and got != names[k])) return 145;
        if (k % 2 != 0) strcpy(names[k], got);
        if (fopen(names[k], "r") != NULL) return 146;
        for (n = 0; n < k; n++) {
            if (strcmp(names[n], names[k]) == 0) return 147;
        }
    }
    if (not put(names[0], "w", "named") or not holds(names[0], "named") or remove(names[0]) != 0) return 148;

    if (freopen("errors", "w", stderr) != stderr) return 149;
    errno = EDOM;
    perror("first");
    errno = ERANGE;
    perror(NULL);
    errno = EILSEQ;
    perror("");
    if (ferror(stderr) or fflush(stderr) != 0) return 150;
    file = fopen("errors", "r");
    if (file == NULL or not reads(file, "first: ", EDOM)) return 151;
    if (not reads(file, "", ERANGE) or not reads(file, "", EILSEQ)) return 152;
    if (fgetc(file) != EOF or fclose(file) != 0) return 153;
    if (fclose(stdin) != 0) return 154;
    return 0;
}
