// (Test) Status: 0
// A test file for <_int.h> characters and digits.

// Include helpers.
#include "_util06_00_int_helpers.inc"

// A digit or a letter of either case should have its value, and no other one.
static void Test_IntDigits(void)
{
    const int wide[] = { 'a', 0x20AC, 0 };

    _TEST_INT(_Int_ToDigit('0'), 0);
    _TEST_INT(_Int_ToDigit('9'), 9);
    _TEST_INT(_Int_ToDigit('a'), 10);
    _TEST_INT(_Int_ToDigit('A'), 10);
    _TEST_INT(_Int_ToDigit('z'), 35);
    _TEST_INT(_Int_ToDigit('Z'), 35);
    _TEST_INT(_Int_ToDigit('/'), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit(':'), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit('@'), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit('['), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit('`'), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit('{'), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit('\0'), _INT_BASE_MAX);
    _TEST_INT(_Int_ToDigit(0x20AC), _INT_BASE_MAX);
    _TEST_TRUE(_Int_IsSpace(' '));
    _TEST_TRUE(_Int_IsSpace('\t'));
    _TEST_TRUE(_Int_IsSpace('\n'));
    _TEST_TRUE(_Int_IsSpace('\v'));
    _TEST_TRUE(_Int_IsSpace('\f'));
    _TEST_TRUE(_Int_IsSpace('\r'));
    _TEST_TRUE(! _Int_IsSpace('\0'));
    _TEST_TRUE(! _Int_IsSpace('\b'));
    _TEST_TRUE(! _Int_IsSpace('0'));
    _TEST_INT(_Int_ReadChar("a\377", _INT_TEXT_NARROW, 0), 'a');
    _TEST_INT(_Int_ReadChar("a\377", _INT_TEXT_NARROW, 1), 255);
    _TEST_INT(_Int_ReadChar(wide, _INT_TEXT_WIDE, 0), 'a');
    _TEST_INT(_Int_ReadChar(wide, _INT_TEXT_WIDE, 1), 0x20AC);
    _TEST_INT(_Int_ReadChar(wide, _INT_TEXT_WIDE, 2), 0);
}

int main(void)
{
    Test_IntDigits();
    return _Test_Status();
}
