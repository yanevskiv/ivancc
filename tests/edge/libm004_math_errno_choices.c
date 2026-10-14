// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | ilogb zero EDOM
// | ilogb infinity EDOM
// | ilogb nan EDOM
// | logb zero 0
// | fmod by zero EDOM
// | fmod of infinity EDOM
// | remainder by zero EDOM
// | remquo of infinity EDOM
// | ldexp to subnormal 0
// | ldexp to zero ERANGE
// | nextafter from zero 0
// | nextafter to subnormal ERANGE
// | lrint out of range 0
// | lround of nan 0
// Non-standard: the exact functions should set errno as glibc's do where C99
// lets them choose, but remquo, which should set EDOM as remainder does.
// Liberty: whether these are domain or range errors, and whether an
// underflow sets ERANGE, is the implementation's.
// [C99:7.12.1p2]:     a domain error, an infinity per footnote 203 if outside
// [C99:7.12.1p5]:     an underflow, ERANGE implementation-defined
// [C99:7.12.6.5p2]:   ilogb, a domain or range error that may occur
// [C99:7.12.6.11p2]:  logb, a domain or range error that may occur of a zero
// [C99:7.12.9.5p2]:   lrint, a domain or range error that may occur out of range
// [C99:7.12.9.7p2]:   lround, the same
// [C99:7.12.10.1p3]:  fmod, by zero a domain error or zero, implementation-defined
// [C99:7.12.10.2p3]:  remainder, the same
// [C99:7.12.10.3p3]:  remquo, the same
// [C99:7.12.11.3p2]:  nextafter, a range error that may occur

#include <errno.h>
#include <math.h>
#include <stdio.h>

// Operands the compiler cannot fold.
static volatile double zero = 0.0;
static volatile double one = 1.0;

// Print what errno holds after a call, and clear it.
static void show(const char *what)
{
    printf("%s %s\n", what, errno == EDOM ? "EDOM" : errno == ERANGE ? "ERANGE" : errno == 0 ? "0" : "other");
    errno = 0;
}

int main(void)
{
    double inf = one / zero;
    double dnan = zero / zero;
    int quo;

    errno = 0;
    ilogb(zero);
    show("ilogb zero");
    ilogb(inf);
    show("ilogb infinity");
    ilogb(dnan);
    show("ilogb nan");
    logb(zero);
    show("logb zero");
    fmod(one, zero);
    show("fmod by zero");
    fmod(inf, one);
    show("fmod of infinity");
    remainder(one, zero);
    show("remainder by zero");
    remquo(inf, one, &quo);
    show("remquo of infinity");
    ldexp(3.0, -1075);
    show("ldexp to subnormal");
    ldexp(one, -1080);
    show("ldexp to zero");
    nextafter(zero, one);
    show("nextafter from zero");
    nextafter(0x1p-1022, zero);
    show("nextafter to subnormal");
    lrint(0x1p63);
    show("lrint out of range");
    lround(dnan);
    show("lround of nan");
    return 0;
}
