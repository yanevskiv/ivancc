// (Test) Status: 0
// <errno.h> declares errno and the macros EDOM, EILSEQ and ERANGE (S7.5).

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>

#if !(EDOM > 0 && EILSEQ > 0 && ERANGE > 0)
#error "errno.h: the macros are not positive"
#endif

#if EDOM == EILSEQ || EDOM == ERANGE || EILSEQ == ERANGE
#error "errno.h: the macros are not distinct"
#endif

static void set(int value)
{
    errno = value;
}

int main(void)
{
    int *where = &errno;

    if (errno != 0) return 1;
    errno = EDOM;
    if (errno != EDOM) return 2;
    set(ERANGE);
    if (errno != ERANGE) return 3;
    *where = EILSEQ;
    if (errno != EILSEQ) return 4;
    errno++;
    if (errno != EILSEQ + 1) return 5;
    errno = 0;
    if (*where != 0) return 6;
    return 0;
}
