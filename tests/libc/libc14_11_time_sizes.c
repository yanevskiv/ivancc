// (Test) Status: 0
// A test file for <time.h> strftime buffer sizes.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99:7.23.3.5p8] strftime should count what it wrote, nothing if too long.
static void Test_TimeSizes(void)
{
    time_t when = LAST_1999;
    struct tm tm = *gmtime(&when);
    char buf[TEXT_SIZE];

    _TEST_UINT(strftime(buf, 5, "%Y", &tm), 4);
    _TEST_STR(buf, "1999");
    _TEST_UINT(strftime(buf, 4, "%Y", &tm), 0);
    _TEST_UINT(strftime(buf, 1, "", &tm), 0);
    _TEST_INT(buf[0], '\0');
    _TEST_UINT(strftime(buf, sizeof(buf), "a%%b%nc", &tm), 5);
    _TEST_STR(buf, "a%b\nc");
}

int main(void)
{
    Test_TimeSizes();
    return _Test_Status();
}
