// (Test) Status: 0
// A test file for <_fp.h> characters, digits and words.

// Include helpers.
#include "_util08_00_fp_helpers.inc"

// A character should be classed and converted as in "C", a word in any case.
static void Test_FpCharacters(void)
{
    const int wide[] = { 'I', 'n', 'F', 0x20AC, 0 };

    _TEST_TRUE(_Fp_IsSpace(' '));
    _TEST_TRUE(_Fp_IsSpace('\t'));
    _TEST_TRUE(_Fp_IsSpace('\r'));
    _TEST_TRUE(! _Fp_IsSpace('\0'));
    _TEST_TRUE(! _Fp_IsSpace('1'));
    _TEST_TRUE(_Fp_IsAlnum('0'));
    _TEST_TRUE(_Fp_IsAlnum('9'));
    _TEST_TRUE(_Fp_IsAlnum('a'));
    _TEST_TRUE(_Fp_IsAlnum('Z'));
    _TEST_TRUE(! _Fp_IsAlnum('_'));
    _TEST_TRUE(! _Fp_IsAlnum(')'));
    _TEST_TRUE(! _Fp_IsAlnum(0x20AC));
    _TEST_INT(_Fp_ToLower('A'), 'a');
    _TEST_INT(_Fp_ToLower('Z'), 'z');
    _TEST_INT(_Fp_ToLower('a'), 'a');
    _TEST_INT(_Fp_ToLower('@'), '@');
    _TEST_INT(_Fp_ToLower('['), '[');
    _TEST_INT(_Fp_ToHexDigit('0'), 0);
    _TEST_INT(_Fp_ToHexDigit('9'), 9);
    _TEST_INT(_Fp_ToHexDigit('a'), 10);
    _TEST_INT(_Fp_ToHexDigit('F'), 15);
    _TEST_INT(_Fp_ToHexDigit('g'), _FP_HEX_LIMIT);
    _TEST_INT(_Fp_ToHexDigit('G'), _FP_HEX_LIMIT);
    _TEST_INT(_Fp_ToHexDigit('\0'), _FP_HEX_LIMIT);
    _TEST_UINT(_Fp_MatchWord("INFinity", _FP_TEXT_NARROW, 0, "inf"), 3);
    _TEST_UINT(_Fp_MatchWord("INFinity", _FP_TEXT_NARROW, 0, "infinity"), 8);
    _TEST_UINT(_Fp_MatchWord("-nan", _FP_TEXT_NARROW, 1, "nan"), 3);
    _TEST_UINT(_Fp_MatchWord("inx", _FP_TEXT_NARROW, 0, "inf"), 0);
    _TEST_UINT(_Fp_MatchWord("in", _FP_TEXT_NARROW, 0, "inf"), 0);
    _TEST_UINT(_Fp_MatchWord(wide, _FP_TEXT_WIDE, 0, "inf"), 3);
    _TEST_INT(_Fp_ReadChar(wide, _FP_TEXT_WIDE, 3), 0x20AC);
    _TEST_INT(_Fp_ReadChar("\377", _FP_TEXT_NARROW, 0), 255);
}

int main(void)
{
    Test_FpCharacters();
    return _Test_Status();
}
