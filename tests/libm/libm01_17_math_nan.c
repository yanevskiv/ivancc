// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <math.h> nan.

// Include helpers.
#include "libm01_00_math_helpers.inc"

// Defines.
#define TAGS       6
#define BUILT      2
#define LDBL_BYTES 10

// nan, nanf and nanl should give a quiet NaN, the very one strtod, strtof
// and strtold give for "NAN(tagp)", or for "NAN" when tagp is no
// n-char-sequence, and the compiler's NaN builtins the same constant.
// [C99:7.12.11.2p2]: nan, strtod of "NAN(n-char-sequence)", else of "NAN"
// [C99:7.12.11.2p3]: nan, a quiet NaN with content indicated through tagp
// [C99:F.9.8.2p1]:   quiet NaNs, in all floating formats
static void Test_MathNan(void)
{
    static const char *tags[TAGS] = {"", "0x12", "12", "012", "abc", "0x7fffffffffffffff"};
    static const char *texts[TAGS] = {"NAN()", "NAN(0x12)", "NAN(12)", "NAN(012)", "NAN(abc)", "NAN(0x7fffffffffffffff)"};
    static const double built[BUILT] = {__builtin_nan(""), __builtin_nan("0x12")};
    static const float fbuilt[BUILT] = {__builtin_nanf(""), __builtin_nanf("0x12")};
    static const long double lbuilt[BUILT] = {__builtin_nanl(""), __builtin_nanl("0x12")};
    long double lnan = 0.0L;
    long double lread = 0.0L;
    double dnan = 0.0;
    double read = 0.0;
    float fnan = 0.0f;
    float fread = 0.0f;

    feclearexcept(FE_ALL_EXCEPT);
    for (int i = 0; i < TAGS; i++) {
        dnan = nan(tags[i]);
        read = strtod(texts[i], NULL);
        _TEST_TRUE(isnan(dnan));
        _TEST_MEM(&dnan, &read, sizeof(dnan));
        fnan = nanf(tags[i]);
        fread = strtof(texts[i], NULL);
        _TEST_TRUE(isnan(fnan));
        _TEST_MEM(&fnan, &fread, sizeof(fnan));
        lnan = nanl(tags[i]);
        lread = strtold(texts[i], NULL);
        _TEST_TRUE(isnan(lnan));
        _TEST_MEM(&lnan, &lread, LDBL_BYTES);
    }
    dnan = nan("-1");
    read = strtod("NAN", NULL);
    _TEST_MEM(&dnan, &read, sizeof(dnan));
    dnan = nan("1.5");
    _TEST_MEM(&dnan, &read, sizeof(dnan));

    for (int i = 0; i < BUILT; i++) {
        dnan = nan(tags[i]);
        _TEST_MEM(&built[i], &dnan, sizeof(dnan));
        fnan = nanf(tags[i]);
        _TEST_MEM(&fbuilt[i], &fnan, sizeof(fnan));
        lnan = nanl(tags[i]);
        _TEST_MEM(&lbuilt[i], &lnan, LDBL_BYTES);
    }
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
}

int main(void)
{
    Test_MathNan();
    return _Test_Status();
}
