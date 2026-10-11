// (Test) Status: 1
// (Test) Error:
// | _util00_01_test_macros.c:93: Test_TestFails: zero is false
// | _util00_01_test_macros.c:94: Test_TestFails: 1 == 2 is false
// | _util00_01_test_macros.c:95: Test_TestFails: none is false
// | _util00_01_test_macros.c:96: Test_TestFails: -5 is -5, expected 5
// | _util00_01_test_macros.c:97: Test_TestFails: least is -9223372036854775808, expected 0
// | _util00_01_test_macros.c:98: Test_TestFails: 0 is 0, expected 9223372036854775807
// | _util00_01_test_macros.c:99: Test_TestFails: most is 18446744073709551615, expected 0
// | _util00_01_test_macros.c:100: Test_TestFails: sizeof(word) is 4, expected 3
// | _util00_01_test_macros.c:101: Test_TestFails: 1.5 is 0x3fffc000000000000000, expected 0x4000a000000000000000
// | _util00_01_test_macros.c:102: Test_TestFails: 0.1F is 0x3ffbcccccd0000000000, expected 0x3ffbccccccccccccd000
// | _util00_01_test_macros.c:103: Test_TestFails: nought is 0x00000000000000000000, expected 0x80000000000000000000
// | _util00_01_test_macros.c:104: Test_TestFails: infinite is 0x7fff8000000000000000, expected 0x3fff8000000000000000
// | _util00_01_test_macros.c:105: Test_TestFails: third is 0x3ffdaaaaaaaaaaaaaaab, expected 0x3ffd8000000000000000
// | _util00_01_test_macros.c:106: Test_TestFails: infinite is 0x7fff8000000000000000, expected 0xffff8000000000000000
// | _util00_01_test_macros.c:107: Test_TestFails: invalid is 0xffffc000000000000000, expected 0x7fff8000000000000000
// | _util00_01_test_macros.c:108: Test_TestFails: word is "abc", expected "abd"
// | _util00_01_test_macros.c:109: Test_TestFails: word is "abc", expected "ab"
// | _util00_01_test_macros.c:110: Test_TestFails: "" is "", expected "a"
// | _util00_01_test_macros.c:111: Test_TestFails: none is a null pointer, expected ""
// | _util00_01_test_macros.c:112: Test_TestFails: word is "abc", expected a null pointer
// | _util00_01_test_macros.c:113: Test_TestFails: odd is "a\"b\\c\012\177\377 ~", expected ""
// | _util00_01_test_macros.c:114: Test_TestFails: bytes is "ab\000c", expected "ab\000d"
// | _util00_01_test_macros.c:115: Test_TestFails: word is "abc\000", expected "abc\001"
// | _util00_01_test_macros.c:126: main: _Test_Status() is 1, expected 0
// A test file for <_test.h> check macros.

// Includes.
#include <_test.h>

#define PASSES 19
#define FIRST  1

static int calls;

// Return the number of calls made so far, this one counted.
static int next(void)
{
    calls++;
    return calls;
}

// A check with operands that pass should return 1: count those that do.
static int Test_TestPasses(void)
{
    static const char bytes[] = {'a', 'b', '\0', 'c'};
    static const char word[] = "abc";
    volatile double nought = 0.0;
    double invalid = nought / nought;
    double infinite = 1 / nought;
    const char *none = 0;
    int kept = 0;

    kept += _TEST_TRUE(1);
    kept += _TEST_TRUE(word);
    kept += _TEST_TRUE(word[0] == 'a' && bytes[3] == 'c');
    kept += _TEST_INT(-5, -5);
    kept += _TEST_INT(-9223372036854775807L - 1, -9223372036854775807L - 1);
    kept += _TEST_UINT(sizeof(word), 4);
    kept += _TEST_UINT(18446744073709551615UL, 18446744073709551615UL);
    kept += _TEST_FLOAT(1.5F, 1.5);
    kept += _TEST_FLOAT(0.1, 0.1);
    kept += _TEST_FLOAT(1.0L / 3, 1.0L / 3);
    kept += _TEST_FLOAT(-nought, -0.0);
    kept += _TEST_FLOAT(infinite, infinite);
    kept += _TEST_FLOAT(invalid, -invalid);
    kept += _TEST_STR(word, "abc");
    kept += _TEST_STR("", "");
    kept += _TEST_STR(none, none);
    kept += _TEST_MEM(bytes, "ab\0c", sizeof(bytes));
    kept += _TEST_MEM(word, "abd", 2);
    kept += _TEST_MEM(word, none, 0);
    return kept;
}

// A check with operands that fail should return 0: count those that do not.
static int Test_TestFails(void)
{
    static const char bytes[] = {'a', 'b', '\0', 'c'};
    static const char odd[] = "a\"b\\c\n\177\377 ~";
    static const char word[] = "abc";
    unsigned long most = 18446744073709551615UL;
    long least = -9223372036854775807L - 1;
    long double third = 1.0L / 3;
    volatile double nought = 0.0;
    double invalid = nought / nought;
    double infinite = 1 / nought;
    const char *none = 0;
    int zero = 0;
    int lost = 0;

    lost += _TEST_TRUE(zero);
    lost += _TEST_TRUE(1 == 2);
    lost += _TEST_TRUE(none);
    lost += _TEST_INT(-5, 5);
    lost += _TEST_INT(least, 0);
    lost += _TEST_INT(0, 9223372036854775807L);
    lost += _TEST_UINT(most, 0);
    lost += _TEST_UINT(sizeof(word), 3);
    lost += _TEST_FLOAT(1.5, 2.5);
    lost += _TEST_FLOAT(0.1F, 0.1);
    lost += _TEST_FLOAT(nought, -nought);
    lost += _TEST_FLOAT(infinite, 1);
    lost += _TEST_FLOAT(third, 0.25);
    lost += _TEST_FLOAT(infinite, -infinite);
    lost += _TEST_FLOAT(invalid, infinite);
    lost += _TEST_STR(word, "abd");
    lost += _TEST_STR(word, "ab");
    lost += _TEST_STR("", "a");
    lost += _TEST_STR(none, "");
    lost += _TEST_STR(word, none);
    lost += _TEST_STR(odd, "");
    lost += _TEST_MEM(bytes, "ab\0d", sizeof(bytes));
    lost += _TEST_MEM(word, "abc\1", sizeof(word));
    return lost;
}

int main(void)
{
    _TEST_INT(Test_TestPasses(), PASSES);
    _TEST_INT(_Test_Status(), 0);
    _TEST_INT(next(), FIRST);
    _TEST_INT(calls, FIRST);
    _TEST_INT(Test_TestFails(), 0);
    _TEST_INT(_Test_Status(), 0);
    return _Test_Status();
}
