// (Test) Status: 0
// A test file for <time.h> calls by pointer and past a macro.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99:7.1.4p1] A pointer and a call in parentheses should reach each function.
static void Test_TimeCalls(void)
{
    time_t when = 0;
    struct tm tm = make(1973, 8, 16, 1, 3, 52);
    char buf[TEXT_SIZE];
    char *(*text)(const struct tm *) = asctime;
    size_t (*format)(char *restrict, size_t, const char *restrict, const struct tm *restrict) = strftime;

    tm.tm_wday = 0;
    _TEST_STR(text(&tm), "Sun Sep 16 01:03:52 1973\n");
    _TEST_UINT(format(buf, sizeof(buf), "%T", &tm), 8);
    _TEST_STR(buf, "01:03:52");

    _TEST_TRUE((clock)() != (clock_t) -1);
    _TEST_FLOAT((difftime)(2, 1), 1.0);
    _TEST_TRUE((mktime)(&tm) != (time_t) -1);
    _TEST_TRUE((time)(NULL) != (time_t) -1);
    _TEST_STR((asctime)(&tm), "Sun Sep 16 01:03:52 1973\n");
    strcpy(buf, (ctime)(&when));
    _TEST_STR(asctime(localtime(&when)), buf);
    _TEST_INT((gmtime)(&when)->tm_year, 70);
    _TEST_TRUE((localtime)(&when) != NULL);
    _TEST_UINT((strftime)(buf, sizeof(buf), "%Y", &tm), 4);
    _TEST_STR(buf, "1973");
}

int main(void)
{
    Test_TimeCalls();
    return _Test_Status();
}
