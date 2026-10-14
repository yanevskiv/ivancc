// (Test) Status: 0
// A test file for <wchar.h> wide character input.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"

// A wide character got should be the stream's next, or WEOF.
// [C99:7.24.3.1]: fgetwc
static void Test_WcharWideInput(void)
{
    FILE *file = holding(NAME, "abcd", 4);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(getwc(file) == L'b');
    _TEST_TRUE((fgetwc)(file) == L'c');
    _TEST_TRUE((getwc)(file) == L'd');
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_TRUE(feof(file));
    _TEST_TRUE(! ferror(file));
    _TEST_TRUE(getwc(file) == WEOF);
    _TEST_INT(fclose(file), 0);

    file = fopen(NAME, "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_TRUE(ferror(file));
    _TEST_TRUE(! feof(file));
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

// fgetws should read a line, n - 1 wide characters at most.
// [C99:7.24.3.2]: fgetws
static void Test_WcharWideLines(void)
{
    wchar_t line[SIZE];
    FILE *file = holding(NAME, "ab\ncdefg\n\nhi", 12);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    put(line, L"xxxxxxxx", FILLED);
    _TEST_TRUE(fgetws(line, SIZE, file) == line);
    _TEST_MEM(line, L"ab\n\0xxxx", WIDE(8));
    _TEST_TRUE(fgetws(line, 4, file) == line);
    _TEST_MEM(line, L"cde\0xxxx", WIDE(8));
    _TEST_TRUE(fgetws(line, 1, file) == line);
    _TEST_MEM(line, L"\0de\0xxxx", WIDE(8));
    _TEST_TRUE((fgetws)(line, SIZE, file) == line);
    _TEST_MEM(line, L"fg\n", WIDE(4));
    _TEST_TRUE(fgetws(line, SIZE, file) == line);
    _TEST_MEM(line, L"\n", WIDE(2));
    _TEST_TRUE(fgetws(line, SIZE, file) == line);
    _TEST_MEM(line, L"hi", WIDE(3));
    _TEST_TRUE(feof(file));
    put(line, L"xxxxxxxx", FILLED);
    _TEST_TRUE(fgetws(line, SIZE, file) == NULL);
    _TEST_MEM(line, L"xxxxxxxx", WIDE(8));
    _TEST_TRUE(! ferror(file));
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

// ungetwc should give a wide character to the next read, once.
// [C99:7.24.3.10]: ungetwc
static void Test_WcharWidePushback(void)
{
    wchar_t line[SIZE];
    FILE *file = holding(NAME, "abc", 3);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(ungetwc(L'x', file) == L'x');
    _TEST_TRUE(fgetwc(file) == L'x');
    _TEST_TRUE(fgetwc(file) == L'b');
    _TEST_TRUE(ungetwc(WEOF, file) == WEOF);
    _TEST_TRUE(fgetwc(file) == L'c');
    _TEST_TRUE(fgetwc(file) == WEOF);
    _TEST_TRUE(feof(file));
    _TEST_TRUE((ungetwc)(L'y', file) == L'y');
    _TEST_TRUE(! feof(file));
    _TEST_TRUE(fgetws(line, SIZE, file) == line);
    _TEST_MEM(line, L"y", WIDE(2));
    _TEST_TRUE(feof(file));

    rewind(file);
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(ungetwc(L'z', file) == L'z');
    _TEST_INT(fseek(file, 1, SEEK_SET), 0);
    _TEST_TRUE(fgetwc(file) == L'b');
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

int main(void)
{
    Test_WcharWideInput();
    Test_WcharWideLines();
    Test_WcharWidePushback();
    return _Test_Status();
}
