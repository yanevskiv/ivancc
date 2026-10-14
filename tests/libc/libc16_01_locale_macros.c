// (Test) Status: 0
// A test file for <locale.h> macros.

// Include helpers.
#include "libc16_00_locale_helpers.inc"

// Each category should differ from the next in #if.
// [C99:7.11p3]: NULL and the LC_ categories, distinct
#define DIRECTIVE 0
#if (LC_ALL != LC_COLLATE) \
    && (LC_COLLATE != LC_CTYPE) \
    && (LC_CTYPE != LC_MONETARY) \
    && (LC_MONETARY != LC_NUMERIC) \
    && (LC_NUMERIC != LC_TIME)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

// The categories should be distinct: return the first to repeat.
// [C99:7.11p3]: NULL and the LC_ categories, distinct
static int repeated(void)
{
    int i;
    int j;

    for (i = 0; i < CATEGORIES; i++) {
        for (j = 0; j < i; j++) {
            if (cats[i] == cats[j]) {
                return i;
            }
        }
    }
    return NONE;
}

// The categories should be distinct constants, and NULL null.
// [C99:7.11p3]: NULL and the LC_ categories, distinct
static void Test_LocaleMacros(void)
{
    _TEST_INT(DIRECTIVE, 1);
    _TEST_INT(repeated(), NONE);
    _TEST_TRUE(NULL == (void *) 0);
}

int main(void)
{
    Test_LocaleMacros();
    return _Test_Status();
}
