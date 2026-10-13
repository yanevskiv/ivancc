// (Test) Status: 0
// A test file for <_utf8.h> codes that do not encode.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

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
    lost += ! _TEST_INT(_Utf8_GetCodeLength(code), 0);
    lost += ! _TEST_INT(_Utf8_EncodeCode(code, buf), _UTF8_INVALID);
    lost += ! _TEST_MEM(buf, want, SIZE);
    return lost == 0;
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

int main(void)
{
    Test_Utf8Uncoded();
    return _Test_Status();
}
