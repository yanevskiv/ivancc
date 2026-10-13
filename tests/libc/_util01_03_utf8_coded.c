// (Test) Status: 0
// A test file for <_utf8.h> codes that encode.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

// A code should encode as the len bytes given, and they decode to it.
static _Bool coded(unsigned int code, const char *bytes, int len)
{
    int k;
    int lost = 0;
    unsigned int back = UNSET;
    unsigned char buf[SIZE];
    unsigned char want[SIZE];

    for (k = 0; k < SIZE; k++) {
        want[k] = k < len ? (unsigned char) bytes[k] : FILL;
        buf[k] = FILL;
    }
    lost += ! _TEST_INT(_Utf8_GetCodeLength(code), len);
    lost += ! _TEST_INT(_Utf8_EncodeCode(code, buf), len);
    lost += ! _TEST_MEM(buf, want, SIZE);
    lost += ! _TEST_INT(_Utf8_GetLeadLength(want[0]), len);
    lost += ! _TEST_INT(_Utf8_DecodeSequence(want, SIZE, &back), len);
    lost += ! _TEST_UINT(back, code);
    for (k = 0; k < len; k++) {
        lost += ! refused(bytes, (unsigned long) k, _UTF8_INCOMPLETE);
    }
    return lost == 0;
}

// Each length's first and last code, and some between, should code both ways.
static void Test_Utf8Coded(void)
{
    _TEST_TRUE(coded(0x00, "\x00", 1));
    _TEST_TRUE(coded(0x41, "A", 1));
    _TEST_TRUE(coded(0x7F, "\x7F", 1));
    _TEST_TRUE(coded(0x80, "\xC2\x80", 2));
    _TEST_TRUE(coded(0xE9, "\xC3\xA9", 2));
    _TEST_TRUE(coded(0x7FF, "\xDF\xBF", 2));
    _TEST_TRUE(coded(0x800, "\xE0\xA0\x80", 3));
    _TEST_TRUE(coded(0x20AC, "\xE2\x82\xAC", 3));
    _TEST_TRUE(coded(0xD7FF, "\xED\x9F\xBF", 3));
    _TEST_TRUE(coded(0xE000, "\xEE\x80\x80", 3));
    _TEST_TRUE(coded(0xFFFF, "\xEF\xBF\xBF", 3));
    _TEST_TRUE(coded(0x10000, "\xF0\x90\x80\x80", 4));
    _TEST_TRUE(coded(0x1F600, "\xF0\x9F\x98\x80", 4));
    _TEST_TRUE(coded(0x100000, "\xF4\x80\x80\x80", 4));
    _TEST_TRUE(coded(0x10FFFF, "\xF4\x8F\xBF\xBF", 4));
}

int main(void)
{
    Test_Utf8Coded();
    return _Test_Status();
}
