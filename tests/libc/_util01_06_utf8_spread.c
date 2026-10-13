// (Test) Status: 0
// A test file for <_utf8.h> codes of every size.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

#define SPREAD 64

// Codes of every size, each a 64th past the last, should code both ways.
static void Test_Utf8Spread(void)
{
    int len;
    unsigned int back;
    unsigned int code = 0;
    unsigned char buf[_UTF8_MAX];
    unsigned long none = 0;
    unsigned long wrong = 0;

    while (code <= _UTF8_CODE_MAX) {
        len = _Utf8_GetCodeLength(code);
        back = UNSET;
        if (len == 0) {
            none++;
            wrong += _Utf8_EncodeCode(code, buf) != _UTF8_INVALID;
        } else {
            wrong += _Utf8_EncodeCode(code, buf) != len;
            wrong += _Utf8_DecodeSequence(buf, (unsigned long) len, &back) != len;
            wrong += back != code;
        }
        code += code / SPREAD + 1;
    }
    _TEST_UINT(wrong, 0);
    _TEST_TRUE(none > 0);
}

int main(void)
{
    Test_Utf8Spread();
    return _Test_Status();
}
