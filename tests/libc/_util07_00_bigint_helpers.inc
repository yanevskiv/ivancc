// Includable helper file for <_bigint.h> tests.

// Includes.
#include <_test.h>
#include <_bigint.h>

// Defines.
#define WORD_MAX 0xFFFFFFFFU
#define LONG_ALL 0xFFFFFFFFFFFFFFFFULL

// A number should be len words, its lowest low and its highest high.
static _Bool holds(const _Bigint_Number *num, int len, unsigned int low, unsigned int high)
{
    int lost = 0;

    lost += ! _TEST_INT(num->bn_len, len);
    if (len > 0 && num->bn_len == len) {
        lost += ! _TEST_UINT(num->bn_word[0], low);
        lost += ! _TEST_UINT(num->bn_word[len - 1], high);
    }
    return lost == 0;
}
