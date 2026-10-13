// (Test) Status: 0
// A test file for <_utf8.h> lead byte lengths.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

// A lead byte should tell its sequence's length, and any other byte 0.
static void Test_Utf8LeadLength(void)
{
    _TEST_INT(_Utf8_GetLeadLength(0x00), 1);
    _TEST_INT(_Utf8_GetLeadLength(0x7F), 1);
    _TEST_INT(_Utf8_GetLeadLength(0x80), 0);
    _TEST_INT(_Utf8_GetLeadLength(0xBF), 0);
    _TEST_INT(_Utf8_GetLeadLength(0xC0), 0);
    _TEST_INT(_Utf8_GetLeadLength(0xC1), 0);
    _TEST_INT(_Utf8_GetLeadLength(0xC2), 2);
    _TEST_INT(_Utf8_GetLeadLength(0xDF), 2);
    _TEST_INT(_Utf8_GetLeadLength(0xE0), 3);
    _TEST_INT(_Utf8_GetLeadLength(0xEF), 3);
    _TEST_INT(_Utf8_GetLeadLength(0xF0), 4);
    _TEST_INT(_Utf8_GetLeadLength(0xF7), 4);
    _TEST_INT(_Utf8_GetLeadLength(0xF8), 5);
    _TEST_INT(_Utf8_GetLeadLength(0xFB), 5);
    _TEST_INT(_Utf8_GetLeadLength(0xFC), 6);
    _TEST_INT(_Utf8_GetLeadLength(0xFD), 6);
    _TEST_INT(_Utf8_GetLeadLength(0xFE), 0);
    _TEST_INT(_Utf8_GetLeadLength(0xFF), 0);
}

int main(void)
{
    Test_Utf8LeadLength();
    return _Test_Status();
}
