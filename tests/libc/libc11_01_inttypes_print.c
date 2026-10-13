// (Test) Status: 0
// A test file for <inttypes.h> PRI macros.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99:S7.8.1p1] Each PRI macro should be its type's length modifier and letter.
static void Test_InttypesPrint(void)
{
    _TEST_STR(PRId8, "d");
    _TEST_STR(PRId16, "d");
    _TEST_STR(PRId32, "d");
    _TEST_STR(PRId64, "ld");
    _TEST_STR(PRIdLEAST8, "d");
    _TEST_STR(PRIdLEAST16, "d");
    _TEST_STR(PRIdLEAST32, "d");
    _TEST_STR(PRIdLEAST64, "ld");
    _TEST_STR(PRIdFAST8, "d");
    _TEST_STR(PRIdFAST16, "ld");
    _TEST_STR(PRIdFAST32, "ld");
    _TEST_STR(PRIdFAST64, "ld");
    _TEST_STR(PRIdMAX, "ld");
    _TEST_STR(PRIdPTR, "ld");

    _TEST_STR(PRIi8, "i");
    _TEST_STR(PRIi16, "i");
    _TEST_STR(PRIi32, "i");
    _TEST_STR(PRIi64, "li");
    _TEST_STR(PRIiLEAST8, "i");
    _TEST_STR(PRIiLEAST16, "i");
    _TEST_STR(PRIiLEAST32, "i");
    _TEST_STR(PRIiLEAST64, "li");
    _TEST_STR(PRIiFAST8, "i");
    _TEST_STR(PRIiFAST16, "li");
    _TEST_STR(PRIiFAST32, "li");
    _TEST_STR(PRIiFAST64, "li");
    _TEST_STR(PRIiMAX, "li");
    _TEST_STR(PRIiPTR, "li");

    _TEST_STR(PRIo8, "o");
    _TEST_STR(PRIo16, "o");
    _TEST_STR(PRIo32, "o");
    _TEST_STR(PRIo64, "lo");
    _TEST_STR(PRIoLEAST8, "o");
    _TEST_STR(PRIoLEAST16, "o");
    _TEST_STR(PRIoLEAST32, "o");
    _TEST_STR(PRIoLEAST64, "lo");
    _TEST_STR(PRIoFAST8, "o");
    _TEST_STR(PRIoFAST16, "lo");
    _TEST_STR(PRIoFAST32, "lo");
    _TEST_STR(PRIoFAST64, "lo");
    _TEST_STR(PRIoMAX, "lo");
    _TEST_STR(PRIoPTR, "lo");

    _TEST_STR(PRIu8, "u");
    _TEST_STR(PRIu16, "u");
    _TEST_STR(PRIu32, "u");
    _TEST_STR(PRIu64, "lu");
    _TEST_STR(PRIuLEAST8, "u");
    _TEST_STR(PRIuLEAST16, "u");
    _TEST_STR(PRIuLEAST32, "u");
    _TEST_STR(PRIuLEAST64, "lu");
    _TEST_STR(PRIuFAST8, "u");
    _TEST_STR(PRIuFAST16, "lu");
    _TEST_STR(PRIuFAST32, "lu");
    _TEST_STR(PRIuFAST64, "lu");
    _TEST_STR(PRIuMAX, "lu");
    _TEST_STR(PRIuPTR, "lu");

    _TEST_STR(PRIx8, "x");
    _TEST_STR(PRIx16, "x");
    _TEST_STR(PRIx32, "x");
    _TEST_STR(PRIx64, "lx");
    _TEST_STR(PRIxLEAST8, "x");
    _TEST_STR(PRIxLEAST16, "x");
    _TEST_STR(PRIxLEAST32, "x");
    _TEST_STR(PRIxLEAST64, "lx");
    _TEST_STR(PRIxFAST8, "x");
    _TEST_STR(PRIxFAST16, "lx");
    _TEST_STR(PRIxFAST32, "lx");
    _TEST_STR(PRIxFAST64, "lx");
    _TEST_STR(PRIxMAX, "lx");
    _TEST_STR(PRIxPTR, "lx");

    _TEST_STR(PRIX8, "X");
    _TEST_STR(PRIX16, "X");
    _TEST_STR(PRIX32, "X");
    _TEST_STR(PRIX64, "lX");
    _TEST_STR(PRIXLEAST8, "X");
    _TEST_STR(PRIXLEAST16, "X");
    _TEST_STR(PRIXLEAST32, "X");
    _TEST_STR(PRIXLEAST64, "lX");
    _TEST_STR(PRIXFAST8, "X");
    _TEST_STR(PRIXFAST16, "lX");
    _TEST_STR(PRIXFAST32, "lX");
    _TEST_STR(PRIXFAST64, "lX");
    _TEST_STR(PRIXMAX, "lX");
    _TEST_STR(PRIXPTR, "lX");

    _TEST_STR("%" PRId64 "|%" PRIXPTR, "%ld|%lX");
}

int main(void)
{
    Test_InttypesPrint();
    return _Test_Status();
}
