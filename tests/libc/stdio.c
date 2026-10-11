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
// | first: Numerical argument out of domain
// | Numerical result out of range
// | Invalid or incomplete multibyte or wide character
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
// its end-of-file indicator staying set until clearerr or ungetc clears it (S7.19.7.1p3, S7.19.7.11p5),
// each of fopen's fifteen modes (S7.19.5.3p3), an update stream writing after a flush or a read to the end of its file (S7.19.5.3p6),
// FOPEN_MAX streams open at once (S7.19.1p3), freopen giving a stream another file and clearing its indicators (S7.19.5.4p4),
// a binary stream's position counted in characters from its file's start (S7.19.9.4p2) and one less after ungetc (S7.19.7.11p5),
// fseek dropping a pushed-back character and the end-of-file indicator (S7.19.9.2p5), a text stream returning to what ftell gave (S7.19.9.2p4),
// an update stream reading and writing either side of a positioning call (S7.19.5.3p6), an append stream writing at its file's end wherever it is positioned (S7.19.5.3p5),
// a removed file opening no more (S7.19.4.1p2) and a renamed one by its new name alone (S7.19.4.2p2), tmpfile's streams each reading back what it was written (S7.19.4.3p2),
// tmpnam giving a different name each call (S7.19.4.4p3), one no file has (S7.19.4.4p2), in its array or its own (S7.19.4.4p5),
// and perror writing its string, a colon and a space, where it has one, then strerror's text and a new-line to stderr (S7.19.10.4p2).

#include <_test.h>
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


// (S7.19.1p3) The macros should be distinct and within their limits in #if.
#define DIRECTIVE 0
#if defined(NULL) \
    && (_IOFBF != _IOLBF) \
    && (_IOFBF != _IONBF) \
    && (_IOLBF != _IONBF) \
    && (BUFSIZ >= 256) \
    && (EOF < 0) \
    && (FOPEN_MAX >= 8) \
    && (FILENAME_MAX >= 1) \
    && (L_tmpnam >= 1) \
    && (TMP_MAX >= 25) \
    && (SEEK_CUR != SEEK_END) \
    && (SEEK_CUR != SEEK_SET) \
    && (SEEK_END != SEEK_SET)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

// (S7.19.1p3) File streams stderr, stdin and stdout should be macros.
#define STREAMS 0
#if defined(stderr) \
    && defined(stdin) \
    && defined(stdout)
#undef STREAMS
#define STREAMS 1
#endif

#define NONE   (-1)
#define SMALL  16
#define LINE   32
#define HOLD   64
#define PRIME  251
#define BIG    (3 * BUFSIZ + 5)
#define WIDE   (BUFSIZ + 100)
#define MANY   (FOPEN_MAX - 3)
#define MODES  15
#define NAMES  25
#define BYTES  8
#define CTRL_Z 26
#define LONGER "a line longer than the buffer it passes through\n"
#define DIGITS "0123456789abcdefghijklmnopqrstuvwxyz"

// Bytes a text stream might change, which a binary one must not.
static const unsigned char bytes[BYTES] = {0, UCHAR_MAX, '\n', '\r', '\n', CTRL_Z, 'a', 0};

// fopen's modes, the last of which leaves its file empty.
static const char *const modes[MODES] = {"r", "rb", "r+", "rb+", "r+b", "a", "ab", "a+", "ab+", "a+b", "w", "wb", "w+", "wb+", "w+b"};

static char outbuf[SMALL];
static char inbuf[SMALL];
static char filebuf[SMALL];
static unsigned char wide[WIDE];
static char names[NAMES][L_tmpnam];

// Write a line as exit runs, after all that main wrote.
static void last(void)
{
    puts("atexit's");
}

// (S7.19.5.3p3) A file opened in a mode should take a string and close.
static bool put(const char *name, const char *mode, const char *str)
{
    int lost = 0;
    FILE *file = fopen(name, mode);

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_TRUE(fputs(str, file) >= 0);
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// A file should hold a string and no more.
static bool holds(const char *name, const char *str)
{
    int lost = 0;
    size_t len;
    FILE *file = fopen(name, "r");
    char got[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    len = fread(got, 1, sizeof(got) - 1, file);
    got[len] = '\0';
    lost += ! _TEST_STR(got, str);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_TRUE(! ferror(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// (S7.19.7.2p2) fgets should read a stream's next line, or what is left.
static bool reads(FILE *file, const char *want)
{
    int lost = 0;
    char got[HOLD];

    got[0] = '\0';
    lost += ! _TEST_TRUE(fgets(got, sizeof(got), file) == got);
    lost += ! _TEST_STR(got, want);
    return lost == 0;
}

// (S7.19.5.3p3) A file should open in each mode: return the first it does not.
static int unopened(void)
{
    int k;
    FILE *file;

    for (k = 0; k < MODES; k++) {
        file = fopen("file", modes[k]);
        if (file == NULL || fclose(file) != 0) {
            return k;
        }
    }
    return NONE;
}

// (S7.19.7.3p3) fputc should write BIG bytes: return the first it fails.
static long unwritten(FILE *file)
{
    long k;

    for (k = 0; k < BIG; k++) {
        if (fputc((int) (k % PRIME), file) == EOF) {
            return k;
        }
    }
    return NONE;
}

// (S7.19.7.1p2) fgetc should read BIG bytes back: return the first to differ.
static long misread(FILE *file)
{
    long k;

    for (k = 0; k < BIG; k++) {
        if (fgetc(file) != (int) (k % PRIME)) {
            return k;
        }
    }
    return NONE;
}

// Give each byte of the wide block a value its place alone decides.
static void pattern(void)
{
    size_t k;

    for (k = 0; k < sizeof(wide); k++) {
        wide[k] = (unsigned char) (k % PRIME);
    }
}

// The wide block should hold its pattern: return the first byte that does not.
static long unpatterned(void)
{
    size_t k;

    for (k = 0; k < sizeof(wide); k++) {
        if (wide[k] != k % PRIME) {
            return (long) k;
        }
    }
    return NONE;
}

// (S7.19.3p15) MANY files should open at once: return the first to fail.
static int crowd(void)
{
    int k;
    char name[] = "many?";
    FILE *many[MANY];

    for (k = 0; k < MANY; k++) {
        name[sizeof(name) - 2] = DIGITS[k];
        many[k] = fopen(name, "w");
        if (many[k] == NULL || fputc(DIGITS[k], many[k]) == EOF) {
            return k;
        }
    }
    for (k = 1; k < MANY; k += 2) {
        if (fclose(many[k]) != 0) {
            return k;
        }
    }
    for (k = 0; k < MANY; k += 2) {
        if (fclose(many[k]) != 0) {
            return k;
        }
    }
    return NONE;
}

// Each of the MANY files should hold its digit: return the first without.
static int uncrowded(void)
{
    int k;
    char name[] = "many?";
    char digit[2];

    for (k = 0; k < MANY; k++) {
        name[sizeof(name) - 2] = DIGITS[k];
        digit[0] = DIGITS[k];
        digit[1] = '\0';
        if (! holds(name, digit)) {
            return k;
        }
    }
    return NONE;
}

// (S7.19.5.6p2) A stream buffered one way should pass lines past its size.
static bool buffers(int kind)
{
    int lost = 0;
    char *buf = kind == _IONBF ? NULL : filebuf;
    FILE *file = fopen("file", "w");

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! _TEST_TRUE(fputs(LONGER, file) >= 0);
    lost += ! _TEST_TRUE(fputs("two\n", file) >= 0);
    lost += ! _TEST_INT(fclose(file), 0);
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_INT(setvbuf(file, buf, kind, sizeof(filebuf)), 0);
    lost += ! reads(file, LONGER);
    lost += ! reads(file, "two\n");
    lost += ! _TEST_INT(fgetc(file), EOF);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// (S7.19.4.4p3) tmpnam should give NAMES new names: return the first not new.
static int unnamed(void)
{
    int k;
    int n;
    const char *got;

    for (k = 0; k < NAMES; k++) {
        got = tmpnam(k % 2 == 0 ? names[k] : NULL);
        if (got == NULL || strlen(got) >= L_tmpnam || (k % 2 == 0 && got != names[k])) {
            return k;
        }
        if (k % 2 != 0) {
            strcpy(names[k], got);
        }
        if (fopen(names[k], "r") != NULL) {
            return k;
        }
        for (n = 0; n < k; n++) {
            if (strcmp(names[n], names[k]) == 0) {
                return k;
            }
        }
    }
    return NONE;
}

// (S7.19.1p3) The macros should be defined and within their limits.
static void Test_StdioMacros(void)
{
    _TEST_INT(DIRECTIVE, 1);
    _TEST_INT(STREAMS, 1);
    _TEST_TRUE(NULL == (void *) 0);
    _TEST_TRUE(_IOFBF != _IOLBF && _IOFBF != _IONBF && _IOLBF != _IONBF);
    _TEST_TRUE(BUFSIZ >= 256);
    _TEST_TRUE(EOF < 0);
    _TEST_TRUE(FOPEN_MAX >= 8);
    _TEST_TRUE(FILENAME_MAX >= 1);
    _TEST_TRUE(L_tmpnam >= 1);
    _TEST_TRUE(TMP_MAX >= NAMES);
    _TEST_TRUE(SEEK_CUR != SEEK_END && SEEK_CUR != SEEK_SET && SEEK_END != SEEK_SET);
    _TEST_TRUE(sizeof(FILE) > 0);
    _TEST_TRUE(sizeof(fpos_t) > 0);
    _TEST_TRUE(sizeof(LONGER) > SMALL);
}

// (S7.19.3p7) The three standard streams should be three streams.
static void Test_StdioStreams(void)
{
    FILE *in = stdin;
    FILE *out = stdout;
    FILE *err = stderr;

    _TEST_TRUE(in != NULL);
    _TEST_TRUE(out != NULL);
    _TEST_TRUE(err != NULL);
    _TEST_TRUE(in != out);
    _TEST_TRUE(in != err);
    _TEST_TRUE(out != err);
}

// (S7.19.3p3) Each output function should write through a smaller buffer.
static void Test_StdioOutput(void)
{
    int bad = 0;

    while (bad == _IOFBF || bad == _IOLBF || bad == _IONBF) {
        bad++;
    }
    _TEST_TRUE(setvbuf(stdout, NULL, bad, 0) != 0);
    _TEST_INT(setvbuf(stdout, outbuf, _IOFBF, sizeof(outbuf)), 0);
    setbuf(stderr, NULL);
    _TEST_INT(atexit(last), 0);

    _TEST_INT(putchar('a'), 'a');
    _TEST_INT(putc('b', stdout), 'b');
    _TEST_INT(fputc('c' + UCHAR_MAX + 1, stdout), 'c');
    _TEST_INT(fputc('\n', stdout), '\n');
    _TEST_TRUE(fputs("fputs, ", stdout) >= 0);
    _TEST_TRUE(puts("puts") >= 0);
    _TEST_UINT(fwrite("fwrite ", 1, 7, stdout), 7);
    _TEST_UINT(fwrite("0123456789", 5, 2, stdout), 2);
    _TEST_UINT(fwrite("x", 0, 3, stdout), 0);
    _TEST_UINT(fwrite("x", 1, 0, stdout), 0);
    _TEST_INT(putchar('\n'), '\n');
    _TEST_TRUE(fputs(LONGER, stdout) >= 0);
    _TEST_INT(fflush(stdout), 0);
    _TEST_TRUE(fputs("after the flush\n", stdout) >= 0);
    _TEST_INT(fflush(NULL), 0);

    _TEST_TRUE(fputs("unbuffered\n", stderr) >= 0);
    _TEST_INT(fflush(stderr), 0);
}

// (S7.19.3p3) Each input function should read through a small buffer.
static void Test_StdioInput(void)
{
    char line[LINE];
    char block[HOLD];

    _TEST_INT(setvbuf(stdin, inbuf, _IOFBF, sizeof(inbuf)), 0);
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(! ferror(stdin));
    _TEST_INT(getchar(), 'a');
    _TEST_INT(getc(stdin), 'b');
    _TEST_INT(fgetc(stdin), 'c');
    _TEST_INT(ungetc(EOF, stdin), EOF);
    _TEST_INT(ungetc('x' + UCHAR_MAX + 1, stdin), 'x');
    _TEST_INT(fgetc(stdin), 'x');
    _TEST_INT(fgetc(stdin), '\n');
    _TEST_TRUE(fgets(line, 8, stdin) == line);
    _TEST_STR(line, "a line ");
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == line);
    _TEST_STR(line, "read in two parts\n");
    _TEST_TRUE(gets(line) == line);
    _TEST_STR(line, "gets takes this one");
    _TEST_UINT(fread(block, 4, 9, stdin), 9);
    _TEST_MEM(block, DIGITS, 36);
    _TEST_UINT(fread(block, 0, 3, stdin), 0);
    _TEST_UINT(fread(block, 1, 0, stdin), 0);
    _TEST_INT(fgetc(stdin), '\n');
    _TEST_UINT(fread(block, 7, 3, stdin), 3);
    _TEST_MEM(block, "a block over a refill", 21);
    _TEST_INT(ungetc(UCHAR_MAX, stdin), UCHAR_MAX);
    _TEST_INT(fgetc(stdin), UCHAR_MAX);
    _TEST_INT(fgetc(stdin), '\n');
    _TEST_TRUE(! feof(stdin));
    _TEST_UINT(fread(block, 3, 2, stdin), 1);
    _TEST_MEM(block, "end", 3);
    _TEST_TRUE(feof(stdin));
    _TEST_TRUE(! ferror(stdin));

    _TEST_INT(fgetc(stdin), EOF);
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == NULL);
    _TEST_STR(line, "gets takes this one");
    _TEST_TRUE(gets(line) == NULL);
    clearerr(stdin);
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(! ferror(stdin));
    _TEST_INT(fgetc(stdin), EOF);
    _TEST_TRUE(feof(stdin));
    _TEST_INT(ungetc('z', stdin), 'z');
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(fgets(line, sizeof(line), stdin) == line);
    _TEST_STR(line, "z");
    _TEST_TRUE(feof(stdin));
}

// (S7.19.5.3p3) fopen should make, extend and empty a file, and read none lost.
static void Test_StdioOpening(void)
{
    FILE *file;

    _TEST_TRUE(fopen("missing", "r") == NULL);
    _TEST_TRUE(fopen("missing", "rb") == NULL);
    _TEST_TRUE(fopen("missing", "r+") == NULL);
    file = fopen("file", "w");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(file != stdin && file != stdout && file != stderr);
    _TEST_TRUE(fputs("abcdef\n", file) >= 0);
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("file", "abcdef\n"));
    _TEST_TRUE(put("file", "a", "gh\n"));
    _TEST_TRUE(holds("file", "abcdef\ngh\n"));
    _TEST_TRUE(put("file", "w", "abcdef"));
    _TEST_TRUE(holds("file", "abcdef"));
    _TEST_TRUE(put("made by a", "a", "a"));
    _TEST_TRUE(put("made by w+", "w+", "w+"));
    _TEST_TRUE(put("made by a+", "a+", "a+"));
    _TEST_TRUE(holds("made by a", "a"));
    _TEST_TRUE(holds("made by w+", "w+"));
    _TEST_TRUE(holds("made by a+", "a+"));
}

// (S7.19.5.3p6) Update streams should write after a flush or a read to the end.
static void Test_StdioUpdate(void)
{
    FILE *file = fopen("file", "r+");
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs("XY", file) >= 0);
    _TEST_INT(fflush(file), 0);
    _TEST_INT(fgetc(file), 'c');
    _TEST_UINT(fread(block, 1, 8, file), 3);
    _TEST_MEM(block, "def", 3);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(fputs("gh", file) >= 0);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("file", "XYcdefgh"));

    file = fopen("file", "w+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs("hello", file) >= 0);
    _TEST_INT(fflush(file), 0);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    _TEST_INT(fclose(file), 0);

    file = fopen("file", "a+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs(", world", file) >= 0);
    _TEST_INT(fflush(file), 0);
    _TEST_INT(fgetc(file), EOF);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("file", "hello, world"));

    _TEST_INT(unopened(), NONE);
    _TEST_TRUE(holds("file", ""));
}

// (S7.19.2p3) A binary stream should keep every byte, past its buffer's size.
static void Test_StdioBinary(void)
{
    FILE *file = fopen("bytes", "wb");
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_UINT(fwrite(bytes, 1, sizeof(bytes), file), sizeof(bytes));
    _TEST_INT(fclose(file), 0);
    file = fopen("bytes", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_UINT(fread(block, 1, sizeof(block), file), sizeof(bytes));
    _TEST_MEM(block, bytes, sizeof(bytes));
    _TEST_TRUE(feof(file));
    _TEST_INT(fclose(file), 0);

    file = fopen("big", "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(unwritten(file), NONE);
    _TEST_INT(fclose(file), 0);
    file = fopen("big", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(misread(file), NONE);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);

    pattern();
    file = fopen("big", "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fputc('!', file), '!');
    _TEST_UINT(fwrite(wide, sizeof(wide), 1, file), 1);
    _TEST_INT(fclose(file), 0);
    memset(wide, 0, sizeof(wide));
    file = fopen("big", "rb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fgetc(file), '!');
    _TEST_UINT(fread(wide, 1, sizeof(wide), file), sizeof(wide));
    _TEST_INT(unpatterned(), NONE);
    _TEST_INT(fgetc(file), EOF);
    _TEST_INT(fclose(file), 0);
}

// (S7.19.3p15) FOPEN_MAX streams should be open at once, each its own file's.
static void Test_StdioMany(void)
{
    _TEST_INT(crowd(), NONE);
    _TEST_INT(uncrowded(), NONE);
}

// (S7.19.5.6p2) A stream should write and read the same however buffered.
static void Test_StdioBuffering(void)
{
    _TEST_TRUE(buffers(_IOFBF));
    _TEST_TRUE(buffers(_IOLBF));
    _TEST_TRUE(buffers(_IONBF));
}

// (S7.19.5.4p2) freopen should give a stream another file, indicators cleared.
static void Test_StdioFreopen(void)
{
    FILE *file = fopen("file", "w");
    char line[LINE];

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputs("first", file) >= 0);
    if (! _TEST_TRUE(freopen("other", "w", file) == file)) {
        return;
    }
    _TEST_TRUE(fputs("second", file) >= 0);
    if (! _TEST_TRUE(freopen("file", "r", file) == file)) {
        return;
    }
    _TEST_TRUE(reads(file, "first"));
    _TEST_TRUE(feof(file));
    if (! _TEST_TRUE(freopen("other", "a+", file) == file)) {
        return;
    }
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_TRUE(fputs("!", file) >= 0);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("other", "second!"));

    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fgetc(file), 'f');
    _TEST_INT(ungetc('?', file), '?');
    if (! _TEST_TRUE(freopen("other", "r", file) == file)) {
        return;
    }
    _TEST_TRUE(reads(file, "second!"));
    _TEST_TRUE(freopen("missing", "r", file) == NULL);

    _TEST_TRUE(feof(stdin));
    if (! _TEST_TRUE(freopen("other", "r", stdin) == stdin)) {
        return;
    }
    _TEST_TRUE(! feof(stdin));
    _TEST_TRUE(! ferror(stdin));
    _TEST_INT(getchar(), 's');
    _TEST_TRUE(gets(line) == line);
    _TEST_STR(line, "econd!");
}

// (S7.19.9.4p2) A binary stream's position should count characters.
static void Test_StdioPositions(void)
{
    fpos_t pos;
    FILE *file = fopen("pos", "w+b");
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(ftell(file), 0);
    _TEST_UINT(fwrite(DIGITS, 1, 36, file), 36);
    _TEST_INT(ftell(file), 36);
    rewind(file);
    _TEST_INT(ftell(file), 0);
    _TEST_INT(fgetc(file), '0');
    _TEST_INT(ftell(file), 1);
    _TEST_INT(fseek(file, 10, SEEK_SET), 0);
    _TEST_INT(fgetc(file), 'a');
    _TEST_INT(ftell(file), 11);
    _TEST_INT(fseek(file, 5, SEEK_CUR), 0);
    _TEST_INT(ftell(file), 16);
    _TEST_INT(fgetc(file), 'g');
    _TEST_INT(fseek(file, -7, SEEK_CUR), 0);
    _TEST_INT(ftell(file), 10);
    _TEST_INT(fgetc(file), 'a');

    _TEST_INT(ungetc('?', file), '?');
    _TEST_INT(ftell(file), 10);
    _TEST_INT(fseek(file, 0, SEEK_CUR), 0);
    _TEST_INT(fgetc(file), 'a');
    _TEST_UINT(fread(block, 1, sizeof(block), file), 25);
    _TEST_TRUE(feof(file));
    _TEST_INT(fseek(file, 0, SEEK_CUR), 0);
    _TEST_TRUE(! feof(file));
    _TEST_INT(ftell(file), 36);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    rewind(file);
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fgetc(file), '0');
    _TEST_TRUE(fseek(file, -1, SEEK_SET) != 0);

    _TEST_INT(fseek(file, 10, SEEK_SET), 0);
    _TEST_UINT(fread(block, 1, 2, file), 2);
    _TEST_MEM(block, "ab", 2);
    _TEST_INT(fseek(file, 0, SEEK_CUR), 0);
    _TEST_UINT(fwrite("XY", 1, 2, file), 2);
    _TEST_INT(ftell(file), 14);
    _TEST_INT(fseek(file, -4, SEEK_CUR), 0);
    _TEST_UINT(fread(block, 1, 6, file), 6);
    _TEST_MEM(block, "abXYef", 6);
    _TEST_INT(fgetpos(file, &pos), 0);
    _TEST_INT(fgetc(file), 'g');
    _TEST_INT(fgetc(file), 'h');
    _TEST_INT(fsetpos(file, &pos), 0);
    _TEST_INT(fgetc(file), 'g');
    _TEST_INT(ftell(file), 17);
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("pos", "0123456789abXYefghijklmnopqrstuvwxyz"));
}

// (S7.19.9.2p4) A text stream should return to where ftell said it was.
static void Test_StdioTextPositions(void)
{
    long at;
    FILE *file;

    _TEST_TRUE(put("pos", "w", "one\ntwo\nthree\n"));
    file = fopen("pos", "r+");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(reads(file, "one\n"));
    at = ftell(file);
    _TEST_TRUE(at >= 0);
    _TEST_TRUE(reads(file, "two\n"));
    _TEST_TRUE(reads(file, "three\n"));
    _TEST_INT(fseek(file, at, SEEK_SET), 0);
    _TEST_TRUE(reads(file, "two\n"));
    _TEST_INT(fseek(file, 0, SEEK_END), 0);
    _TEST_INT(fgetc(file), EOF);
    _TEST_TRUE(feof(file));
    _TEST_INT(fseek(file, 0, SEEK_END), 0);
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(fputs("four\n", file) >= 0);
    _TEST_INT(fseek(file, at, SEEK_SET), 0);
    _TEST_TRUE(reads(file, "two\n"));
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("pos", "one\ntwo\nthree\nfour\n"));
}

// (S7.19.5.3p5) An append stream should write at its file's end, wherever set.
static void Test_StdioAppend(void)
{
    FILE *file;

    _TEST_TRUE(put("pos", "w", "abc"));
    file = fopen("pos", "a");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fseek(file, 0, SEEK_SET), 0);
    _TEST_TRUE(fputs("def", file) >= 0);
    _TEST_INT(fclose(file), 0);
    file = fopen("pos", "a+b");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fseek(file, 1, SEEK_SET), 0);
    _TEST_INT(fgetc(file), 'b');
    _TEST_INT(fseek(file, 0, SEEK_SET), 0);
    _TEST_INT(fputc('g', file), 'g');
    _TEST_INT(fclose(file), 0);
    _TEST_TRUE(holds("pos", "abcdefg"));
}

// (S7.19.9.2p2) fseek should move within a stream's buffer and past it.
static void Test_StdioSeeking(void)
{
    FILE *file = fopen("big", "rb");

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fseek(file, BUFSIZ + 50, SEEK_SET), 0);
    _TEST_INT(fgetc(file), (BUFSIZ + 49) % PRIME);
    _TEST_INT(ftell(file), BUFSIZ + 51);
    _TEST_INT(fseek(file, -(long) BUFSIZ, SEEK_CUR), 0);
    _TEST_INT(fgetc(file), 50);
    _TEST_INT(fseek(file, 3, SEEK_CUR), 0);
    _TEST_INT(fgetc(file), 54);
    _TEST_INT(ftell(file), 56);
    _TEST_INT(fclose(file), 0);
}

// (S7.19.4.2p2) A removed or renamed file should open by its old name no more.
static void Test_StdioFiles(void)
{
    _TEST_TRUE(remove("missing") != 0);
    _TEST_TRUE(rename("missing", "other name") != 0);
    _TEST_TRUE(put("old", "w", "moved"));
    _TEST_INT(rename("old", "new"), 0);
    _TEST_TRUE(fopen("old", "r") == NULL);
    _TEST_TRUE(holds("new", "moved"));
    _TEST_INT(remove("new"), 0);
    _TEST_TRUE(fopen("new", "r") == NULL);
    _TEST_TRUE(remove("new") != 0);
}

// (S7.19.4.3p2) Each of tmpfile's streams should update a file of its own.
static void Test_StdioTmpfile(void)
{
    FILE *file = tmpfile();
    FILE *other = tmpfile();
    char block[HOLD];

    if (! _TEST_TRUE(file != NULL && other != NULL)) {
        return;
    }
    _TEST_TRUE(file != other);
    _TEST_UINT(fwrite(bytes, 1, sizeof(bytes), file), sizeof(bytes));
    _TEST_TRUE(fputs("the other's", other) >= 0);
    rewind(file);
    rewind(other);
    _TEST_UINT(fread(block, 1, sizeof(block), file), sizeof(bytes));
    _TEST_MEM(block, bytes, sizeof(bytes));
    _TEST_TRUE(reads(other, "the other's"));
    _TEST_INT(fseek(file, 2, SEEK_SET), 0);
    _TEST_INT(fputc('!', file), '!');
    _TEST_INT(fseek(file, 2, SEEK_SET), 0);
    _TEST_INT(fgetc(file), '!');
    _TEST_INT(fclose(file), 0);
    _TEST_INT(fclose(other), 0);
}

// (S7.19.4.4p3) tmpnam should give a new name each call, one a file can take.
static void Test_StdioTmpnam(void)
{
    _TEST_INT(unnamed(), NONE);
    _TEST_TRUE(put(names[0], "w", "named"));
    _TEST_TRUE(holds(names[0], "named"));
    _TEST_INT(remove(names[0]), 0);
}

// (S7.19.10.4p2) perror should write its string and errno's text to stderr.
static void Test_StdioPerror(void)
{
    errno = EDOM;
    perror("first");
    errno = ERANGE;
    perror(NULL);
    errno = EILSEQ;
    perror("");
    _TEST_TRUE(! ferror(stderr));
    _TEST_INT(fflush(stderr), 0);
}

// (S7.19.5.1p2) A standard stream should close as any other does.
static void Test_StdioClosing(void)
{
    _TEST_INT(fclose(stdin), 0);
}

int main(void)
{
    Test_StdioMacros();
    Test_StdioStreams();
    Test_StdioOutput();
    Test_StdioInput();
    Test_StdioOpening();
    Test_StdioUpdate();
    Test_StdioBinary();
    Test_StdioMany();
    Test_StdioBuffering();
    Test_StdioFreopen();
    Test_StdioPositions();
    Test_StdioTextPositions();
    Test_StdioAppend();
    Test_StdioSeeking();
    Test_StdioFiles();
    Test_StdioTmpfile();
    Test_StdioTmpnam();
    Test_StdioPerror();
    Test_StdioClosing();
    return _Test_Status();
}
