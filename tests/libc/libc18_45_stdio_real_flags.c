// (Test) Status: 0
// A test file for <stdio.h> flags and widths of floating conversions.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.1p6] A floating conversion should take a width and each flag.
static void Test_StdioRealFlags(void)
{
    _TEST_TRUE(prints("  1.500000", "%10f", 1.5));
    _TEST_TRUE(prints(" -1.500000", "%10f", -1.5));
    _TEST_TRUE(prints("1.500000  |", "%-10f|", 1.5));
    _TEST_TRUE(prints("1.500000", "%3f", 1.5));
    _TEST_TRUE(prints("  1.500000e+00", "%14e", 1.5));
    _TEST_TRUE(prints("1.500000e+00  |", "%-14e|", 1.5));
    _TEST_TRUE(prints("       1.5", "%10g", 1.5));
    _TEST_TRUE(prints("1.5       |", "%-10g|", 1.5));
    _TEST_TRUE(prints("      3.14", "%10.2f", 3.14159));
    _TEST_TRUE(prints("     3e+00", "%10.0e", 3.14159));

    _TEST_TRUE(prints("001.500000", "%010f", 1.5));
    _TEST_TRUE(prints("-01.500000", "%010f", -1.5));
    _TEST_TRUE(prints("-00001.500", "%010.3f", -1.5));
    _TEST_TRUE(prints("001.500000e+00", "%014e", 1.5));
    _TEST_TRUE(prints("-01.500000e+00", "%014e", -1.5));
    _TEST_TRUE(prints("00000001.5", "%010g", 1.5));
    _TEST_TRUE(prints("-0000001.5", "%010g", -1.5));
    _TEST_TRUE(prints("0000000002", "%010.0f", 1.5));
    _TEST_TRUE(prints("1.500000", "%01f", 1.5));
    _TEST_TRUE(prints("1.500     |", "%-010.3f|", 1.5));

    _TEST_TRUE(prints("+1.500000", "%+f", 1.5));
    _TEST_TRUE(prints("-1.500000", "%+f", -1.5));
    _TEST_TRUE(prints("+0.000000", "%+f", 0.0));
    _TEST_TRUE(prints("-0.000000", "%+f", -0.0));
    _TEST_TRUE(prints("+1.500000e+00", "%+e", 1.5));
    _TEST_TRUE(prints("+1.5", "%+g", 1.5));
    _TEST_TRUE(prints(" 1.500000", "% f", 1.5));
    _TEST_TRUE(prints("-1.500000", "% f", -1.5));
    _TEST_TRUE(prints(" 1.500000e+00", "% e", 1.5));
    _TEST_TRUE(prints(" 1.5", "% g", 1.5));
    _TEST_TRUE(prints("+1.500000", "%+ f", 1.5));
    _TEST_TRUE(prints("+1.500000", "% +f", 1.5));

    _TEST_TRUE(prints("+000001.50", "%+010.2f", 1.5));
    _TEST_TRUE(prints(" 000001.50", "% 010.2f", 1.5));
    _TEST_TRUE(prints("     +1.50", "%+10.2f", 1.5));
    _TEST_TRUE(prints("+1.50     |", "%-+10.2f|", 1.5));
    _TEST_TRUE(prints(" 1.50     |", "%- 10.2f|", 1.5));
    _TEST_TRUE(prints("+01.50e+00", "%+010.2e", 1.5));

    _TEST_TRUE(prints("      3.14", "%*.*f", 10, 2, 3.14159));
    _TEST_TRUE(prints("3.14      |", "%*.*f|", -10, 2, 3.14159));
    _TEST_TRUE(prints("0000003.14", "%0*.*f", 10, 2, 3.14159));
    _TEST_TRUE(prints("  3.141590", "%*f", 10, 3.14159));
}

int main(void)
{
    Test_StdioRealFlags();
    return _Test_Status();
}
