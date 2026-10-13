// (Test) Status: 0
// A test file for <time.h> ctime.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// [C99/S7.23.3.2p2] ctime should write a time as asctime writes its local time.
static void Test_TimeCtime(void)
{
    time_t now = time(NULL);
    time_t when = 0;
    char buf[TEXT_SIZE];

    strcpy(buf, ctime(&when));
    _TEST_STR(asctime(localtime(&when)), buf);
    strcpy(buf, ctime(&now));
    _TEST_STR(asctime(localtime(&now)), buf);
}

int main(void)
{
    Test_TimeCtime();
    return _Test_Status();
}
