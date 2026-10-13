// (Test) Status: 0
// A test file for <iso646.h> macro spellings.

// Include helpers.
#include "libc01_00_iso646_helpers.inc"

#define TEXT(token)  #token
#define SPELL(token) TEXT(token)

// [C99/S7.9p1] Each macro should expand to its operator's token and no more.
static void Test_Iso646Spellings(void)
{
    _TEST_STR(SPELL(and), "&&");
    _TEST_STR(SPELL(and_eq), "&=");
    _TEST_STR(SPELL(bitand), "&");
    _TEST_STR(SPELL(bitor), "|");
    _TEST_STR(SPELL(compl), "~");
    _TEST_STR(SPELL(not), "!");
    _TEST_STR(SPELL(not_eq), "!=");
    _TEST_STR(SPELL(or), "||");
    _TEST_STR(SPELL(or_eq), "|=");
    _TEST_STR(SPELL(xor), "^");
    _TEST_STR(SPELL(xor_eq), "^=");
}

int main(void)
{
    Test_Iso646Spellings();
    return _Test_Status();
}
