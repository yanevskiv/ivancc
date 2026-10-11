// (Test) Status: 1
// (Test) Error:
// | _test.c:96: Test_TestFails: zero is false
// | _test.c:97: Test_TestFails: 1 == 2 is false
// | _test.c:98: Test_TestFails: none is false
// | _test.c:99: Test_TestFails: -5 is -5, expected 5
// | _test.c:100: Test_TestFails: least is -9223372036854775808, expected 0
// | _test.c:101: Test_TestFails: 0 is 0, expected 9223372036854775807
// | _test.c:102: Test_TestFails: most is 18446744073709551615, expected 0
// | _test.c:103: Test_TestFails: sizeof(word) is 4, expected 3
// | _test.c:104: Test_TestFails: 1.5 is 0x3fffc000000000000000, expected 0x4000a000000000000000
// | _test.c:105: Test_TestFails: 0.1F is 0x3ffbcccccd0000000000, expected 0x3ffbccccccccccccd000
// | _test.c:106: Test_TestFails: nought is 0x00000000000000000000, expected 0x80000000000000000000
// | _test.c:107: Test_TestFails: infinite is 0x7fff8000000000000000, expected 0x3fff8000000000000000
// | _test.c:108: Test_TestFails: third is 0x3ffdaaaaaaaaaaaaaaab, expected 0x3ffd8000000000000000
// | _test.c:109: Test_TestFails: infinite is 0x7fff8000000000000000, expected 0xffff8000000000000000
// | _test.c:110: Test_TestFails: invalid is 0xffffc000000000000000, expected 0x7fff8000000000000000
// | _test.c:111: Test_TestFails: word is "abc", expected "abd"
// | _test.c:112: Test_TestFails: word is "abc", expected "ab"
// | _test.c:113: Test_TestFails: "" is "", expected "a"
// | _test.c:114: Test_TestFails: none is a null pointer, expected ""
// | _test.c:115: Test_TestFails: word is "abc", expected a null pointer
// | _test.c:116: Test_TestFails: odd is "a\"b\\c\012\177\377 ~", expected ""
// | _test.c:117: Test_TestFails: bytes is "ab\000c", expected "ab\000d"
// | _test.c:118: Test_TestFails: word is "abc\000", expected "abc\001"
// | _test.c:129: main: _Test_Status() is 1, expected 0
// <_test.h> defines _TEST_TRUE, _TEST_INT, _TEST_UINT, _TEST_FLOAT, _TEST_STR and _TEST_MEM, and declares _Test_Status,
// a check that holds printing nothing and returning 1, one that does not returning 0 and reporting its file, line, function and expression to standard error,
// with both values but for a condition's, a floating value written as the ten bytes of its long double in hexadecimal, a string or an object as a string literal and a null string as none,
// a floating value matching one of its bytes alone, so a zero no zero of the other sign, but for two that are no numbers, which match,
// each check evaluating its operands once, and the status 0 until a check fails and 1 from then on.

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

// Make each check with operands that pass, and count those that say so.
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

// Make each check with operands that fail, and count those that say so.
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
