// Includable helper file for <_utf8.h> tests.

// Includes.
#include <_test.h>
#include <_utf8.h>

// Defines.
#define SIZE  (_UTF8_MAX + 2)
#define FILL  0x55
#define UNSET 0xFFFFFFFFU

// A decoding of the n bytes given should fail as want, the code left unset.
static _Bool refused(const char *bytes, unsigned long n, int want)
{
    int lost = 0;
    unsigned int code = UNSET;

    lost += ! _TEST_INT(_Utf8_DecodeSequence((const unsigned char *) bytes, n, &code), want);
    lost += ! _TEST_UINT(code, UNSET);
    return lost == 0;
}
