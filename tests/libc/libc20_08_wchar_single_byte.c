// (Test) Status: 0
// A test file for <wchar.h> single-byte and wide character conversions.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define UTF8 "C.UTF-8"

// A basic character should be one byte and the same wide, each way.
// [C99:7.24.6.1]: btowc and wctob
static void Test_WcharSingleByte(void)
{
    int i;
    const char *basic = "\a\b\t\n\v\f\r !\"#%&'()*+,-./09:;<=>?AZ[\\]^_az{|}~";

    for (i = 0; basic[i] != '\0'; i++) {
        _TEST_TRUE(btowc(basic[i]) == (wint_t) basic[i]);
        _TEST_INT(wctob((wint_t) basic[i]), basic[i]);
    }
    _TEST_TRUE(btowc('a') == L'a');
    _TEST_INT(wctob(L'a'), 'a');
    _TEST_TRUE(btowc('\0') == L'\0');
    _TEST_INT(wctob(L'\0'), 0);
    _TEST_TRUE(btowc(EOF) == WEOF);
    _TEST_INT(wctob(WEOF), EOF);
    _TEST_TRUE((btowc)('b') == L'b');
    _TEST_INT((wctob)(L'b'), 'b');
}

// In UTF-8 a byte of a longer character should be no character.
// [C99:7.24.6.1]: btowc and wctob
static void Test_WcharSingleByteUtf8(void)
{
    if (! _TEST_TRUE(setlocale(LC_CTYPE, UTF8) != NULL)) {
        return;
    }
    _TEST_TRUE(btowc('a') == L'a');
    _TEST_INT(wctob(L'a'), 'a');
    _TEST_TRUE(btowc(0xE2) == WEOF);
    _TEST_TRUE(btowc(0x82) == WEOF);
    _TEST_TRUE(btowc(0xFF) == WEOF);
    _TEST_TRUE(btowc(EOF) == WEOF);
    _TEST_INT(wctob(0xE9), EOF);
    _TEST_INT(wctob(0x20AC), EOF);
    _TEST_INT(wctob(WEOF), EOF);
    _TEST_TRUE(setlocale(LC_CTYPE, "C") != NULL);
}

int main(void)
{
    Test_WcharSingleByte();
    Test_WcharSingleByteUtf8();
    return _Test_Status();
}
