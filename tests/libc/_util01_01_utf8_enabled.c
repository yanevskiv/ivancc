// (Test) Status: 0
// A test file for <_utf8.h> the encoding at startup.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

// The locale should encode in ASCII until setlocale says otherwise.
static void Test_Utf8Enabled(void)
{
    _TEST_TRUE(! _Utf8_Enabled);
}

int main(void)
{
    Test_Utf8Enabled();
    return _Test_Status();
}
