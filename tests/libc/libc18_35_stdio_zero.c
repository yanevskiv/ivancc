// (Test) Status: 0
// A test file for <stdio.h> the 0 flag on integers.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.1p6] A 0 flag should pad with zeros, after any sign or base.
static void Test_StdioZero(void)
{
    _TEST_TRUE(prints("00042", "%05d", 42));
    _TEST_TRUE(prints("-0042", "%05d", -42));
    _TEST_TRUE(prints("+0042", "%+05d", 42));
    _TEST_TRUE(prints(" 0042", "% 05d", 42));
    _TEST_TRUE(prints("00000", "%05d", 0));
    _TEST_TRUE(prints("00042", "%05i", 42));
    _TEST_TRUE(prints("00042", "%05u", 42U));
    _TEST_TRUE(prints("00052", "%05o", 42U));
    _TEST_TRUE(prints("0002a", "%05x", 42U));
    _TEST_TRUE(prints("0002A", "%05X", 42U));
    _TEST_TRUE(prints("0x0002a", "%#07x", 42U));
    _TEST_TRUE(prints("0X0002A", "%#07X", 42U));
    _TEST_TRUE(prints("0000052", "%#07o", 42U));
    _TEST_TRUE(prints("12345", "%03d", 12345));
    _TEST_TRUE(prints("-12345", "%03d", -12345));
    _TEST_TRUE(prints("00042", "%0*d", 5, 42));

    _TEST_TRUE(prints("42   ", "%-05d", 42));
    _TEST_TRUE(prints("42   ", "%0-5d", 42));
    _TEST_TRUE(prints("-42  ", "%-05d", -42));
    _TEST_TRUE(prints("42   ", "%0*d", -5, 42));
    _TEST_TRUE(prints("0x2a   ", "%-#07x", 42U));

    _TEST_TRUE(prints("  042", "%05.3d", 42));
    _TEST_TRUE(prints(" -042", "%05.3d", -42));
    _TEST_TRUE(prints("   42", "%05.0d", 42));
    _TEST_TRUE(prints("     ", "%05.0d", 0));
    _TEST_TRUE(prints("   2a", "%05.1x", 42U));
    _TEST_TRUE(prints("  042", "%05.*d", 3, 42));
    _TEST_TRUE(prints("00042", "%05.*d", -1, 42));
}

int main(void)
{
    Test_StdioZero();
    return _Test_Status();
}
