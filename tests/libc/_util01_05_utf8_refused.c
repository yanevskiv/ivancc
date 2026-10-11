// (Test) Status: 0
// A test file for <_utf8.h> sequences that do not decode.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

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

int main(void)
{
    Test_Utf8Refused();
    return _Test_Status();
}
