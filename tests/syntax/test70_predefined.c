// (Test) Return: 200
// Predefined macros. __FILE__ and __LINE__ name the file and line they are
// read on, and __COUNTER__ counts up each time it expands. #line renumbers the
// lines after it and may rename the file, until the file ends. __STDC__,
// __STDC_VERSION__, __DATE__ and __TIME__ are defined before the source is
// read. __func__ is not a macro. A function body names its function with it.

#include "inc/test70_predefined.h"

#define HERE __LINE__
#define LINE_OF(x) __LINE__
#define START 300
#define NAME "macro.c"

int same(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

int ends_with(const char *text, const char *end)
{
    int n = 0;
    int m = 0;

    while (text[n]) n++;
    while (end[m]) m++;
    return n >= m && same(text + n - m, end);
}

const char *function_name(void)
{
    return __func__;
}

int main()
{
    // __LINE__ counts the lines it is read on.
    int first = __LINE__;
    int second = __LINE__;

    if (second != first + 1) return 1;

    // A macro gives the line it is used on, even when its call spans lines.
    int here = HERE;

    if (here != __LINE__ - 2) return 2;

    int call = LINE_OF(
        0
    );

    if (call != __LINE__ - 4) return 3;

    // __FILE__ names the file being read, a header included or the source.
    if (! ends_with(__FILE__, "test70_predefined.c")) return 4;
    if (! ends_with(header_file(), "inc/test70_predefined.h")) return 5;
    if (header_line() != 10) return 6;

    // #line in a header lasts until the header ends.
    if (! same(renamed_file(), "renamed.h")) return 7;
    if (renamed_line() != 507) return 8;
    if (! ends_with(__FILE__, "test70_predefined.c")) return 9;

    // __COUNTER__ starts at 0 and counts up.
    int c0 = __COUNTER__;
    int c1 = __COUNTER__;

    if (c0 != 0 || c1 != 1) return 10;

    // The standard macros are defined before the source.
#if __STDC__ != 1 || __STDC_VERSION__ != 199901L
    return 11;
#endif
#if ! defined __STDC_HOSTED__ || ! defined __x86_64__ || ! defined __LP64__
    return 12;
#endif
    if (sizeof(__DATE__) != 12 || __DATE__[3] != ' ' || __DATE__[6] != ' ') return 13;
    if (sizeof(__TIME__) != 9 || __TIME__[2] != ':' || __TIME__[5] != ':') return 14;

    // __func__ holds the name of the function it is used in.
#ifdef __func__
    return 15;
#endif
    if (! same(__func__, "main") || sizeof(__func__) != 5) return 16;
    if (! same(function_name(), "function_name")) return 17;

    const char *outer = __func__;
    const char *inner = 0;

    {
        inner = __func__;
    }
    if (inner != outer) return 18;

    // #line numbers the next line and keeps the name.
#line 100
    if (__LINE__ != 100) return 19;
    if (! ends_with(__FILE__, "test70_predefined.c")) return 20;

    // A name after the number renames the file.
#line 200 "other.c"
    if (__LINE__ != 200 || ! same(__FILE__, "other.c")) return 21;

    // A name's escapes are read, and __FILE__ writes them again.
#line 1 "dir\\file.c"
    if (! same(__FILE__, "dir\\file.c") || sizeof(__FILE__) != 11) return 22;

    // The operands of #line are macro-expanded.
#line START NAME
    if (__LINE__ != 300 || ! same(__FILE__, "macro.c")) return 23;

    return 200;
}
