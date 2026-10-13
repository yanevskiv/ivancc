// (Test) Status: 0
// A test file for <inttypes.h> SCN macros.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99 S7.8.1p1] Each SCN macro should be its type's length modifier and letter.
static void Test_InttypesScan(void)
{
    _TEST_STR(SCNd8, "hhd");
    _TEST_STR(SCNd16, "hd");
    _TEST_STR(SCNd32, "d");
    _TEST_STR(SCNd64, "ld");
    _TEST_STR(SCNdLEAST8, "hhd");
    _TEST_STR(SCNdLEAST16, "hd");
    _TEST_STR(SCNdLEAST32, "d");
    _TEST_STR(SCNdLEAST64, "ld");
    _TEST_STR(SCNdFAST8, "hhd");
    _TEST_STR(SCNdFAST16, "ld");
    _TEST_STR(SCNdFAST32, "ld");
    _TEST_STR(SCNdFAST64, "ld");
    _TEST_STR(SCNdMAX, "ld");
    _TEST_STR(SCNdPTR, "ld");

    _TEST_STR(SCNi8, "hhi");
    _TEST_STR(SCNi16, "hi");
    _TEST_STR(SCNi32, "i");
    _TEST_STR(SCNi64, "li");
    _TEST_STR(SCNiLEAST8, "hhi");
    _TEST_STR(SCNiLEAST16, "hi");
    _TEST_STR(SCNiLEAST32, "i");
    _TEST_STR(SCNiLEAST64, "li");
    _TEST_STR(SCNiFAST8, "hhi");
    _TEST_STR(SCNiFAST16, "li");
    _TEST_STR(SCNiFAST32, "li");
    _TEST_STR(SCNiFAST64, "li");
    _TEST_STR(SCNiMAX, "li");
    _TEST_STR(SCNiPTR, "li");

    _TEST_STR(SCNo8, "hho");
    _TEST_STR(SCNo16, "ho");
    _TEST_STR(SCNo32, "o");
    _TEST_STR(SCNo64, "lo");
    _TEST_STR(SCNoLEAST8, "hho");
    _TEST_STR(SCNoLEAST16, "ho");
    _TEST_STR(SCNoLEAST32, "o");
    _TEST_STR(SCNoLEAST64, "lo");
    _TEST_STR(SCNoFAST8, "hho");
    _TEST_STR(SCNoFAST16, "lo");
    _TEST_STR(SCNoFAST32, "lo");
    _TEST_STR(SCNoFAST64, "lo");
    _TEST_STR(SCNoMAX, "lo");
    _TEST_STR(SCNoPTR, "lo");

    _TEST_STR(SCNu8, "hhu");
    _TEST_STR(SCNu16, "hu");
    _TEST_STR(SCNu32, "u");
    _TEST_STR(SCNu64, "lu");
    _TEST_STR(SCNuLEAST8, "hhu");
    _TEST_STR(SCNuLEAST16, "hu");
    _TEST_STR(SCNuLEAST32, "u");
    _TEST_STR(SCNuLEAST64, "lu");
    _TEST_STR(SCNuFAST8, "hhu");
    _TEST_STR(SCNuFAST16, "lu");
    _TEST_STR(SCNuFAST32, "lu");
    _TEST_STR(SCNuFAST64, "lu");
    _TEST_STR(SCNuMAX, "lu");
    _TEST_STR(SCNuPTR, "lu");

    _TEST_STR(SCNx8, "hhx");
    _TEST_STR(SCNx16, "hx");
    _TEST_STR(SCNx32, "x");
    _TEST_STR(SCNx64, "lx");
    _TEST_STR(SCNxLEAST8, "hhx");
    _TEST_STR(SCNxLEAST16, "hx");
    _TEST_STR(SCNxLEAST32, "x");
    _TEST_STR(SCNxLEAST64, "lx");
    _TEST_STR(SCNxFAST8, "hhx");
    _TEST_STR(SCNxFAST16, "lx");
    _TEST_STR(SCNxFAST32, "lx");
    _TEST_STR(SCNxFAST64, "lx");
    _TEST_STR(SCNxMAX, "lx");
    _TEST_STR(SCNxPTR, "lx");

    _TEST_STR("%" SCNd8 "|%" SCNxMAX, "%hhd|%lx");
}

int main(void)
{
    Test_InttypesScan();
    return _Test_Status();
}
