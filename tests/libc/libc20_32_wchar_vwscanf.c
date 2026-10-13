// (Test) Status: 0
// (Test) Input:
// | first 12 second
// A test file for <wchar.h> reading by an argument list.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"

// [C99 S7.24.2.8p2] vswscanf should read a wide string as swscanf does, by a list.
static int reads_string(const wchar_t *str, const wchar_t *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vswscanf(str, format, args);
    va_end(args);
    return n;
}

// [C99 S7.24.2.6p2] vfwscanf should read a stream as fwscanf does, by a list.
static int reads_stream(FILE *file, const wchar_t *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vfwscanf(file, format, args);
    va_end(args);
    return n;
}

// [C99 S7.24.2.10p2] vwscanf should read standard input as wscanf does, by a list.
static int reads_input(const wchar_t *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vwscanf(format, args);
    va_end(args);
    return n;
}

// [C99 S7.24.2.6p2] Each function taking a list should read as its fellow does.
static void Test_WcharVwscanf(void)
{
    int i = 0;
    FILE *file;
    int n = NONE;
    char word[SIZE];
    wchar_t wide[SIZE];

    _TEST_INT(reads_string(L"one 2 two", L"%s%d%ls%n", word, &i, wide, &n), 3);
    _TEST_STR(word, "one");
    _TEST_INT(i, 2);
    _TEST_MEM(wide, L"two", WIDE(4));
    _TEST_INT(n, 9);
    _TEST_INT(reads_string(L"", L"%d", &i), EOF);

    file = holding(NAME, "three 4", 7);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(reads_stream(file, L"%ls%d", wide, &i), 2);
    _TEST_MEM(wide, L"three", WIDE(6));
    _TEST_INT(i, 4);
    _TEST_INT(reads_stream(file, L"%d", &i), EOF);
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

// [C99 S7.24.2.12p2] wscanf and vwscanf should read standard input.
static void Test_WcharWscanf(void)
{
    int i = 0;
    char word[SIZE];
    wchar_t wide[SIZE];

    _TEST_INT(fwide(stdin, 0), 0);
    _TEST_INT(wscanf(L"%s%d", word, &i), 2);
    _TEST_TRUE(fwide(stdin, 0) > 0);
    _TEST_STR(word, "first");
    _TEST_INT(i, 12);
    _TEST_INT(reads_input(L"%ls", wide), 1);
    _TEST_MEM(wide, L"second", WIDE(7));
    _TEST_INT(reads_input(L" %ls", wide), EOF);
}

int main(void)
{
    Test_WcharVwscanf();
    Test_WcharWscanf();
    return _Test_Status();
}
