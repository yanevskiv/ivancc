// (Test) Status: 0
// A test file for <time.h> strftime ISO 8601 weeks.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99 S7.23.3.5p5] A year's ends should fall in an ISO 8601 week of either year.
static void Test_TimeWeeks(void)
{
    _TEST_TRUE(formats(1104537600 + 309, "%G-W%V-%u %g %U %W %e %I %p %j", "2004-W53-6 04 00 00  1 12 AM 001"));
    _TEST_TRUE(formats(1230508800 + 43200, "%G-W%V-%u %g %U %W %I %p", "2009-W01-1 09 52 52 12 PM"));
    _TEST_TRUE(formats(1262476800 + 46800, "%G-W%V-%u %g %U %W %w %I %p", "2009-W53-7 09 01 00 0 01 PM"));
    _TEST_TRUE(formats(1104451200, "%G-W%V-%u %U %W %j", "2004-W53-5 52 52 366"));
    _TEST_TRUE(formats(1136073600, "%G-W%V-%u %U %W", "2005-W52-7 01 00"));
    _TEST_TRUE(formats(1167609600, "%G-W%V-%u %U %W", "2007-W01-1 00 01"));
}

int main(void)
{
    Test_TimeWeeks();
    return _Test_Status();
}
