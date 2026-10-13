// (Test) Status: 0
// A test file for <stdio.h> macros.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.1p3] The macros should be distinct and within their limits in #if.
#define DIRECTIVE 0
#if defined(NULL) \
    && (_IOFBF != _IOLBF) \
    && (_IOFBF != _IONBF) \
    && (_IOLBF != _IONBF) \
    && (BUFSIZ >= 256) \
    && (EOF < 0) \
    && (FOPEN_MAX >= 8) \
    && (FILENAME_MAX >= 1) \
    && (L_tmpnam >= 1) \
    && (TMP_MAX >= 25) \
    && (SEEK_CUR != SEEK_END) \
    && (SEEK_CUR != SEEK_SET) \
    && (SEEK_END != SEEK_SET)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

// [C99:S7.19.1p3] File streams stderr, stdin and stdout should be macros.
#define STREAMS 0
#if defined(stderr) \
    && defined(stdin) \
    && defined(stdout)
#undef STREAMS
#define STREAMS 1
#endif

// [C99:S7.19.1p3] The macros should be defined and within their limits.
static void Test_StdioMacros(void)
{
    _TEST_INT(DIRECTIVE, 1);
    _TEST_INT(STREAMS, 1);
    _TEST_TRUE(NULL == (void *) 0);
    _TEST_TRUE(_IOFBF != _IOLBF && _IOFBF != _IONBF && _IOLBF != _IONBF);
    _TEST_TRUE(BUFSIZ >= 256);
    _TEST_TRUE(EOF < 0);
    _TEST_TRUE(FOPEN_MAX >= 8);
    _TEST_TRUE(FILENAME_MAX >= 1);
    _TEST_TRUE(L_tmpnam >= 1);
    _TEST_TRUE(TMP_MAX >= NAMES);
    _TEST_TRUE(SEEK_CUR != SEEK_END && SEEK_CUR != SEEK_SET && SEEK_END != SEEK_SET);
    _TEST_TRUE(sizeof(FILE) > 0);
    _TEST_TRUE(sizeof(fpos_t) > 0);
    _TEST_TRUE(sizeof(LONGER) > SMALL);
}

int main(void)
{
    Test_StdioMacros();
    return _Test_Status();
}
