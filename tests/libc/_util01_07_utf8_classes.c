// (Test) Status: 0
// A test file for <_utf8.h> classes of codes.

// Include helpers.
#include "_util01_00_utf8_helpers.inc"

// A code should be found in the ranges that hold it, at an end or between.
static void Test_Utf8Ranges(void)
{
    static const _Utf8_Table_Range ranges[] = {{0x10, 0x10}, {0x20, 0x2F}, {0x40, 0x41}};

    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 0, 0x10));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 3, 0x0F));
    _TEST_TRUE(_Utf8_IsInRanges(ranges, 3, 0x10));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 3, 0x11));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 3, 0x1F));
    _TEST_TRUE(_Utf8_IsInRanges(ranges, 3, 0x20));
    _TEST_TRUE(_Utf8_IsInRanges(ranges, 3, 0x28));
    _TEST_TRUE(_Utf8_IsInRanges(ranges, 3, 0x2F));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 3, 0x30));
    _TEST_TRUE(_Utf8_IsInRanges(ranges, 3, 0x40));
    _TEST_TRUE(_Utf8_IsInRanges(ranges, 3, 0x41));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 3, 0x42));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 2, 0x40));
    _TEST_TRUE(! _Utf8_IsInRanges(ranges, 3, 0xFFFFFFFFU));
}

// An ASCII code should be in the classes <ctype.h> puts it in.
static void Test_Utf8ClassesAscii(void)
{
    _TEST_TRUE(_Utf8_IsInClass('a', _UTF8_CLASS_ALNUM));
    _TEST_TRUE(_Utf8_IsInClass('7', _UTF8_CLASS_ALNUM));
    _TEST_TRUE(! _Utf8_IsInClass('_', _UTF8_CLASS_ALNUM));
    _TEST_TRUE(_Utf8_IsInClass('Z', _UTF8_CLASS_ALPHA));
    _TEST_TRUE(! _Utf8_IsInClass('7', _UTF8_CLASS_ALPHA));
    _TEST_TRUE(_Utf8_IsInClass(' ', _UTF8_CLASS_BLANK));
    _TEST_TRUE(_Utf8_IsInClass('\t', _UTF8_CLASS_BLANK));
    _TEST_TRUE(! _Utf8_IsInClass('\n', _UTF8_CLASS_BLANK));
    _TEST_TRUE(_Utf8_IsInClass(0x00, _UTF8_CLASS_CNTRL));
    _TEST_TRUE(_Utf8_IsInClass(0x1F, _UTF8_CLASS_CNTRL));
    _TEST_TRUE(_Utf8_IsInClass(0x7F, _UTF8_CLASS_CNTRL));
    _TEST_TRUE(! _Utf8_IsInClass(' ', _UTF8_CLASS_CNTRL));
    _TEST_TRUE(_Utf8_IsInClass('0', _UTF8_CLASS_DIGIT));
    _TEST_TRUE(_Utf8_IsInClass('9', _UTF8_CLASS_DIGIT));
    _TEST_TRUE(! _Utf8_IsInClass('/', _UTF8_CLASS_DIGIT));
    _TEST_TRUE(! _Utf8_IsInClass(':', _UTF8_CLASS_DIGIT));
    _TEST_TRUE(_Utf8_IsInClass('!', _UTF8_CLASS_GRAPH));
    _TEST_TRUE(_Utf8_IsInClass('~', _UTF8_CLASS_GRAPH));
    _TEST_TRUE(! _Utf8_IsInClass(' ', _UTF8_CLASS_GRAPH));
    _TEST_TRUE(_Utf8_IsInClass('q', _UTF8_CLASS_LOWER));
    _TEST_TRUE(! _Utf8_IsInClass('Q', _UTF8_CLASS_LOWER));
    _TEST_TRUE(_Utf8_IsInClass(' ', _UTF8_CLASS_PRINT));
    _TEST_TRUE(_Utf8_IsInClass('~', _UTF8_CLASS_PRINT));
    _TEST_TRUE(! _Utf8_IsInClass('\t', _UTF8_CLASS_PRINT));
    _TEST_TRUE(! _Utf8_IsInClass(0x7F, _UTF8_CLASS_PRINT));
    _TEST_TRUE(_Utf8_IsInClass('_', _UTF8_CLASS_PUNCT));
    _TEST_TRUE(! _Utf8_IsInClass('a', _UTF8_CLASS_PUNCT));
    _TEST_TRUE(! _Utf8_IsInClass('7', _UTF8_CLASS_PUNCT));
    _TEST_TRUE(! _Utf8_IsInClass(' ', _UTF8_CLASS_PUNCT));
    _TEST_TRUE(_Utf8_IsInClass(' ', _UTF8_CLASS_SPACE));
    _TEST_TRUE(_Utf8_IsInClass('\t', _UTF8_CLASS_SPACE));
    _TEST_TRUE(_Utf8_IsInClass('\r', _UTF8_CLASS_SPACE));
    _TEST_TRUE(! _Utf8_IsInClass(0x08, _UTF8_CLASS_SPACE));
    _TEST_TRUE(_Utf8_IsInClass('Q', _UTF8_CLASS_UPPER));
    _TEST_TRUE(! _Utf8_IsInClass('q', _UTF8_CLASS_UPPER));
    _TEST_TRUE(_Utf8_IsInClass('9', _UTF8_CLASS_XDIGIT));
    _TEST_TRUE(_Utf8_IsInClass('F', _UTF8_CLASS_XDIGIT));
    _TEST_TRUE(_Utf8_IsInClass('f', _UTF8_CLASS_XDIGIT));
    _TEST_TRUE(! _Utf8_IsInClass('G', _UTF8_CLASS_XDIGIT));
    _TEST_TRUE(! _Utf8_IsInClass('g', _UTF8_CLASS_XDIGIT));
}

// A code past ASCII should be in the classes Unicode puts it in.
static void Test_Utf8ClassesUnicode(void)
{
    _TEST_TRUE(_Utf8_IsInClass(0x00E9, _UTF8_CLASS_ALPHA));
    _TEST_TRUE(_Utf8_IsInClass(0x00E9, _UTF8_CLASS_LOWER));
    _TEST_TRUE(! _Utf8_IsInClass(0x00E9, _UTF8_CLASS_UPPER));
    _TEST_TRUE(_Utf8_IsInClass(0x00C9, _UTF8_CLASS_UPPER));
    _TEST_TRUE(_Utf8_IsInClass(0x0416, _UTF8_CLASS_UPPER));
    _TEST_TRUE(_Utf8_IsInClass(0x0436, _UTF8_CLASS_LOWER));
    _TEST_TRUE(_Utf8_IsInClass(0x4E2D, _UTF8_CLASS_ALPHA));
    _TEST_TRUE(_Utf8_IsInClass(0x4E2D, _UTF8_CLASS_ALNUM));
    _TEST_TRUE(! _Utf8_IsInClass(0x4E2D, _UTF8_CLASS_UPPER));
    _TEST_TRUE(! _Utf8_IsInClass(0x4E2D, _UTF8_CLASS_LOWER));
    _TEST_TRUE(! _Utf8_IsInClass(0x0660, _UTF8_CLASS_DIGIT));
    _TEST_TRUE(! _Utf8_IsInClass(0xFF11, _UTF8_CLASS_DIGIT));
    _TEST_TRUE(! _Utf8_IsInClass(0xFF21, _UTF8_CLASS_XDIGIT));
    _TEST_TRUE(_Utf8_IsInClass(0x00D7, _UTF8_CLASS_PUNCT));
    _TEST_TRUE(_Utf8_IsInClass(0x20AC, _UTF8_CLASS_PUNCT));
    _TEST_TRUE(_Utf8_IsInClass(0x20AC, _UTF8_CLASS_GRAPH));
    _TEST_TRUE(! _Utf8_IsInClass(0x20AC, _UTF8_CLASS_ALPHA));
    _TEST_TRUE(_Utf8_IsInClass(0x3000, _UTF8_CLASS_SPACE));
    _TEST_TRUE(_Utf8_IsInClass(0x3000, _UTF8_CLASS_BLANK));
    _TEST_TRUE(_Utf8_IsInClass(0x3000, _UTF8_CLASS_PRINT));
    _TEST_TRUE(! _Utf8_IsInClass(0x3000, _UTF8_CLASS_GRAPH));
    _TEST_TRUE(_Utf8_IsInClass(0x2028, _UTF8_CLASS_SPACE));
    _TEST_TRUE(_Utf8_IsInClass(0x2028, _UTF8_CLASS_CNTRL));
    _TEST_TRUE(! _Utf8_IsInClass(0x2028, _UTF8_CLASS_PRINT));
    _TEST_TRUE(_Utf8_IsInClass(0x0085, _UTF8_CLASS_CNTRL));
    _TEST_TRUE(_Utf8_IsInClass(0x1F600, _UTF8_CLASS_GRAPH));
    _TEST_TRUE(_Utf8_IsInClass(0x1F600, _UTF8_CLASS_PUNCT));
}

// A code that is no character should be in no class.
static void Test_Utf8ClassesNone(void)
{
    int class;

    for (class = _UTF8_CLASS_ALNUM; class <= _UTF8_CLASS_XDIGIT; class++) {
        _TEST_TRUE(! _Utf8_IsInClass(0xD800, (_Utf8_Class) class));
        _TEST_TRUE(! _Utf8_IsInClass(0x110000, (_Utf8_Class) class));
        _TEST_TRUE(! _Utf8_IsInClass(_UTF8_CODE_MAX, (_Utf8_Class) class));
        _TEST_TRUE(! _Utf8_IsInClass(0xFFFFFFFFU, (_Utf8_Class) class));
    }
}

int main(void)
{
    Test_Utf8Ranges();
    Test_Utf8ClassesAscii();
    Test_Utf8ClassesUnicode();
    Test_Utf8ClassesNone();
    return _Test_Status();
}
