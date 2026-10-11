// (Test) Status: 0
// <_utf8.h> declares _Utf8_LeadLength, _Utf8_CodeLength, _Utf8_Decode and _Utf8_Encode, and _Utf8_Enabled, false at startup,
// a code up to 31 bits encoding in the fewest of one to six bytes and decoding back, no surrogate and nothing larger encoding at all,
// a sequence cut short decoding as incomplete, and a stray or missing continuation byte, a lead byte of none, an overlong form and a surrogate as invalid,
// neither a failed decoding nor a failed encoding writing anything.

#include <_test.h>
#include <_utf8.h>

#define SIZE   (_UTF8_MAX + 2)
#define FILL   0x55
#define UNSET  0xFFFFFFFFU
#define SPREAD 64

// A decoding of the n bytes given should fail as want, the code left unset.
static _Bool refused(const char *bytes, unsigned long n, int want)
{
    int lost = 0;
    unsigned int code = UNSET;

    lost += ! _TEST_INT(_Utf8_Decode((const unsigned char *) bytes, n, &code), want);
    lost += ! _TEST_UINT(code, UNSET);
    return lost == 0;
}

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
    lost += ! _TEST_INT(_Utf8_CodeLength(code), len);
    lost += ! _TEST_INT(_Utf8_Encode(code, buf), len);
    lost += ! _TEST_MEM(buf, want, SIZE);
    lost += ! _TEST_INT(_Utf8_LeadLength(want[0]), len);
    lost += ! _TEST_INT(_Utf8_Decode(want, SIZE, &back), len);
    lost += ! _TEST_UINT(back, code);
    for (k = 0; k < len; k++) {
        lost += ! refused(bytes, (unsigned long) k, _UTF8_INCOMPLETE);
    }
    return lost == 0;
}

// A code should encode as nothing, and write nothing.
static _Bool uncoded(unsigned int code)
{
    int k;
    int lost = 0;
    unsigned char buf[SIZE];
    unsigned char want[SIZE];

    for (k = 0; k < SIZE; k++) {
        want[k] = FILL;
        buf[k] = FILL;
    }
    lost += ! _TEST_INT(_Utf8_CodeLength(code), 0);
    lost += ! _TEST_INT(_Utf8_Encode(code, buf), _UTF8_INVALID);
    lost += ! _TEST_MEM(buf, want, SIZE);
    return lost == 0;
}

// The locale should encode in ASCII until setlocale says otherwise.
static void Test_Utf8Enabled(void)
{
    _TEST_TRUE(! _Utf8_Enabled);
}

// A lead byte should tell its sequence's length, and any other byte 0.
static void Test_Utf8LeadLength(void)
{
    _TEST_INT(_Utf8_LeadLength(0x00), 1);
    _TEST_INT(_Utf8_LeadLength(0x7F), 1);
    _TEST_INT(_Utf8_LeadLength(0x80), 0);
    _TEST_INT(_Utf8_LeadLength(0xBF), 0);
    _TEST_INT(_Utf8_LeadLength(0xC0), 0);
    _TEST_INT(_Utf8_LeadLength(0xC1), 0);
    _TEST_INT(_Utf8_LeadLength(0xC2), 2);
    _TEST_INT(_Utf8_LeadLength(0xDF), 2);
    _TEST_INT(_Utf8_LeadLength(0xE0), 3);
    _TEST_INT(_Utf8_LeadLength(0xEF), 3);
    _TEST_INT(_Utf8_LeadLength(0xF0), 4);
    _TEST_INT(_Utf8_LeadLength(0xF7), 4);
    _TEST_INT(_Utf8_LeadLength(0xF8), 5);
    _TEST_INT(_Utf8_LeadLength(0xFB), 5);
    _TEST_INT(_Utf8_LeadLength(0xFC), 6);
    _TEST_INT(_Utf8_LeadLength(0xFD), 6);
    _TEST_INT(_Utf8_LeadLength(0xFE), 0);
    _TEST_INT(_Utf8_LeadLength(0xFF), 0);
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
    _TEST_TRUE(coded(0x10FFFF, "\xF4\x8F\xBF\xBF", 4));
    _TEST_TRUE(coded(0x1FFFFF, "\xF7\xBF\xBF\xBF", 4));
    _TEST_TRUE(coded(0x200000, "\xF8\x88\x80\x80\x80", 5));
    _TEST_TRUE(coded(0x3FFFFFF, "\xFB\xBF\xBF\xBF\xBF", 5));
    _TEST_TRUE(coded(0x4000000, "\xFC\x84\x80\x80\x80\x80", 6));
    _TEST_TRUE(coded(0x7FFFFFFF, "\xFD\xBF\xBF\xBF\xBF\xBF", 6));
}

// A surrogate and a code past 31 bits should have no sequence.
static void Test_Utf8Uncoded(void)
{
    _TEST_TRUE(uncoded(0xD800));
    _TEST_TRUE(uncoded(0xDBFF));
    _TEST_TRUE(uncoded(0xDC00));
    _TEST_TRUE(uncoded(0xDFFF));
    _TEST_TRUE(uncoded(0x80000000U));
    _TEST_TRUE(uncoded(0xFFFFFFFFU));
}

// No bytes should be incomplete, a malformed or overlong sequence invalid.
static void Test_Utf8Refused(void)
{
    _TEST_TRUE(refused("", 0, _UTF8_INCOMPLETE));
    _TEST_TRUE(refused("A", 0, _UTF8_INCOMPLETE));
    _TEST_TRUE(refused("\x80", 1, _UTF8_INVALID));
    _TEST_TRUE(refused("\xBF\x80", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xC0\x80", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xC1\xBF", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xFE\x80", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xFF", 1, _UTF8_INVALID));
    _TEST_TRUE(refused("\xC2\x41", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xC2\xC2", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xE2\x82\x41", 3, _UTF8_INVALID));
    _TEST_TRUE(refused("\xE2\x41\xAC", 2, _UTF8_INVALID));
    _TEST_TRUE(refused("\xF0\x9F\x98\xC0", 4, _UTF8_INVALID));
    _TEST_TRUE(refused("\xE0\x80\x80", 3, _UTF8_INVALID));
    _TEST_TRUE(refused("\xE0\x9F\xBF", 3, _UTF8_INVALID));
    _TEST_TRUE(refused("\xF0\x80\x80\x80", 4, _UTF8_INVALID));
    _TEST_TRUE(refused("\xF0\x8F\xBF\xBF", 4, _UTF8_INVALID));
    _TEST_TRUE(refused("\xF8\x87\xBF\xBF\xBF", 5, _UTF8_INVALID));
    _TEST_TRUE(refused("\xFC\x83\xBF\xBF\xBF\xBF", 6, _UTF8_INVALID));
    _TEST_TRUE(refused("\xED\xA0\x80", 3, _UTF8_INVALID));
    _TEST_TRUE(refused("\xED\xBF\xBF", 3, _UTF8_INVALID));
}

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
        len = _Utf8_CodeLength(code);
        back = UNSET;
        if (len == 0) {
            none++;
            wrong += _Utf8_Encode(code, buf) != _UTF8_INVALID;
        } else {
            wrong += _Utf8_Encode(code, buf) != len;
            wrong += _Utf8_Decode(buf, (unsigned long) len, &back) != len;
            wrong += back != code;
        }
        code += code / SPREAD + 1;
    }
    _TEST_UINT(wrong, 0);
    _TEST_TRUE(none > 0);
}

int main(void)
{
    Test_Utf8Enabled();
    Test_Utf8LeadLength();
    Test_Utf8Coded();
    Test_Utf8Uncoded();
    Test_Utf8Refused();
    Test_Utf8Spread();
    return _Test_Status();
}
