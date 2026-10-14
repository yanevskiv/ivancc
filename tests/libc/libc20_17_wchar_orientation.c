// (Test) Status: 0
// A test file for <wchar.h> fwide and a stream's orientation.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"

// fwide should orient a stream once, and tell its orientation.
// [C99:7.24.3.5p2]: fwide, a stream's orientation
static void Test_WcharOrientation(void)
{
    FILE *file = holding(NAME, "abc", 3);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwide(file, 0), 0);
    _TEST_INT(fwide(file, 0), 0);
    _TEST_TRUE(fwide(file, 1) > 0);
    _TEST_TRUE(fwide(file, 0) > 0);
    _TEST_TRUE(fwide(file, -1) > 0);
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(fwide(file, 0) > 0);

    file = freopen(NAME, "rb", file);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fwide(file, 0), 0);
    _TEST_TRUE(fwide(file, -7) < 0);
    _TEST_TRUE(fwide(file, 7) < 0);
    _TEST_INT(fgetc(file), 'a');
    _TEST_TRUE((fwide)(file, 0) < 0);
    _TEST_INT(fclose(file), 0);
}

// A stream's first read or write should orient it, and no other.
// [C99:7.19.2p4]: a stream's orientation, byte or wide
static void Test_WcharOrientationFirst(void)
{
    FILE *file = holding(NAME, "abc", 3);

    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fgetc(file), 'a');
    _TEST_TRUE(fwide(file, 1) < 0);
    file = freopen(NAME, "rb", file);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fgetwc(file) == L'a');
    _TEST_TRUE(fwide(file, -1) > 0);
    file = freopen(NAME, "wb", file);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(fputc('x', file), 'x');
    _TEST_TRUE(fwide(file, 1) < 0);
    file = freopen(NAME, "wb", file);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputwc(L'x', file) == L'x');
    _TEST_TRUE(fwide(file, -1) > 0);
    file = freopen(NAME, "wb", file);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fprintf(file, "%d", 1) == 1);
    _TEST_TRUE(fwide(file, 1) < 0);
    file = freopen(NAME, "wb", file);
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_TRUE(fputws(L"xy", file) >= 0);
    _TEST_TRUE(fwide(file, -1) > 0);
    _TEST_INT(fclose(file), 0);
    _TEST_INT(remove(NAME), 0);
}

int main(void)
{
    Test_WcharOrientation();
    Test_WcharOrientationFirst();
    return _Test_Status();
}
