// (Test) Status: 0
// A test file for <_int.h> numbers read from text.

// Include helpers.
#include "_util06_00_int_helpers.inc"

// A number should be read past space and a sign, in its base, to its last.
static void Test_IntNumber(void)
{
    const int wide[] = { ' ', '-', '1', '2', '3', 0x20AC, 0 };
    _Int_Sign sign = _INT_SIGN_PLUS;
    _Int_Range range = _INT_RANGE_OUTSIDE;
    unsigned long end = 0;

    _TEST_TRUE(reads("42", 10, 42, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads(" \t\n-42x", 10, 42, 6, _INT_SIGN_MINUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("+7", 10, 7, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("0x1F", 0, 31, 4, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("0X1f", 16, 31, 4, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("1f", 16, 31, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("017", 0, 15, 3, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("019", 0, 1, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("17", 0, 17, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("0x", 0, 0, 1, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("0xg", 16, 0, 1, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("0x10", 10, 0, 1, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("zZ", 36, 1295, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("102", 2, 2, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("", 10, 0, 0, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("abc", 10, 0, 0, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("  -", 10, 0, 0, _INT_SIGN_MINUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("- 1", 10, 0, 0, _INT_SIGN_MINUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("18446744073709551615", 10, _INT_VALUE_MAX, 20, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("0xFFFFFFFFFFFFFFFF", 0, _INT_VALUE_MAX, 18, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_UINT(_Int_ToNumber("18446744073709551616", _INT_TEXT_NARROW, &end, 10, &sign, &range), 1844674407370955161UL);
    _TEST_UINT(end, 20);
    _TEST_INT(range, _INT_RANGE_OUTSIDE);
    _TEST_UINT(_Int_ToNumber(wide, _INT_TEXT_WIDE, &end, 10, &sign, &range), 123);
    _TEST_UINT(end, 5);
    _TEST_INT(sign, _INT_SIGN_MINUS);
    _TEST_INT(range, _INT_RANGE_INSIDE);
}

int main(void)
{
    Test_IntNumber();
    return _Test_Status();
}
