// (Test) Status: 0
// A test file for <_utf8.h> case mappings.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

// A code should be mapped by the shift that holds it, if it is on its steps.
static void Test_Utf8Shifts(void)
{
    static const _Utf8_Table_Shift shifts[] = {{0x10, 0x10, 1, 5}, {0x20, 0x28, 2, -1}, {0x40, 0x42, 1, 0x100}};

    _TEST_UINT(_Utf8_ToShifted(shifts, 0, 0x10), 0x10);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x0F), 0x0F);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x10), 0x15);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x11), 0x11);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x20), 0x1F);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x21), 0x21);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x22), 0x21);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x27), 0x27);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x28), 0x27);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x2A), 0x2A);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x40), 0x140);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x41), 0x141);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x42), 0x142);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0x43), 0x43);
    _TEST_UINT(_Utf8_ToShifted(shifts, 2, 0x40), 0x40);
    _TEST_UINT(_Utf8_ToShifted(shifts, 3, 0xFFFFFFFFU), 0xFFFFFFFFU);
}

// A lower-case letter should map to its upper case, and no other code move.
static void Test_Utf8ToUpper(void)
{
    _TEST_UINT(_Utf8_ToUpper('a'), 'A');
    _TEST_UINT(_Utf8_ToUpper('z'), 'Z');
    _TEST_UINT(_Utf8_ToUpper('A'), 'A');
    _TEST_UINT(_Utf8_ToUpper('`'), '`');
    _TEST_UINT(_Utf8_ToUpper('{'), '{');
    _TEST_UINT(_Utf8_ToUpper('7'), '7');
    _TEST_UINT(_Utf8_ToUpper(0x00E9), 0x00C9);
    _TEST_UINT(_Utf8_ToUpper(0x00FF), 0x0178);
    _TEST_UINT(_Utf8_ToUpper(0x0101), 0x0100);
    _TEST_UINT(_Utf8_ToUpper(0x0100), 0x0100);
    _TEST_UINT(_Utf8_ToUpper(0x03B1), 0x0391);
    _TEST_UINT(_Utf8_ToUpper(0x0436), 0x0416);
    _TEST_UINT(_Utf8_ToUpper(0x10428), 0x10400);
    _TEST_UINT(_Utf8_ToUpper(0x4E2D), 0x4E2D);
    _TEST_UINT(_Utf8_ToUpper(0x110000), 0x110000);
    _TEST_UINT(_Utf8_ToUpper(0xFFFFFFFFU), 0xFFFFFFFFU);
}

// An upper-case letter should map to its lower case, and no other code move.
static void Test_Utf8ToLower(void)
{
    _TEST_UINT(_Utf8_ToLower('A'), 'a');
    _TEST_UINT(_Utf8_ToLower('Z'), 'z');
    _TEST_UINT(_Utf8_ToLower('a'), 'a');
    _TEST_UINT(_Utf8_ToLower('@'), '@');
    _TEST_UINT(_Utf8_ToLower('['), '[');
    _TEST_UINT(_Utf8_ToLower('7'), '7');
    _TEST_UINT(_Utf8_ToLower(0x00C9), 0x00E9);
    _TEST_UINT(_Utf8_ToLower(0x0178), 0x00FF);
    _TEST_UINT(_Utf8_ToLower(0x0100), 0x0101);
    _TEST_UINT(_Utf8_ToLower(0x0101), 0x0101);
    _TEST_UINT(_Utf8_ToLower(0x0391), 0x03B1);
    _TEST_UINT(_Utf8_ToLower(0x0416), 0x0436);
    _TEST_UINT(_Utf8_ToLower(0x10400), 0x10428);
    _TEST_UINT(_Utf8_ToLower(0x4E2D), 0x4E2D);
    _TEST_UINT(_Utf8_ToLower(0x110000), 0x110000);
    _TEST_UINT(_Utf8_ToLower(0xFFFFFFFFU), 0xFFFFFFFFU);
}

// An ASCII letter mapped to its other case should come back by the other.
static void Test_Utf8AsciiRoundTrip(void)
{
    unsigned int code;

    for (code = 'a'; code <= 'z'; code++) {
        _TEST_UINT(_Utf8_ToLower(_Utf8_ToUpper(code)), code);
    }
    for (code = 'A'; code <= 'Z'; code++) {
        _TEST_UINT(_Utf8_ToUpper(_Utf8_ToLower(code)), code);
    }
}

int main(void)
{
    Test_Utf8Shifts();
    Test_Utf8ToUpper();
    Test_Utf8ToLower();
    Test_Utf8AsciiRoundTrip();
    return _Test_Status();
}
