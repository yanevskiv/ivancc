// (Test) Status: 0
// A test file for <time.h> clock.

// Include helpers.
#include "libc14_00_time_helpers.inc"

#define SPINS 100000

// [C99:7.23.2.1p3] The processor time should be known, and never run back.
static void Test_TimeClock(void)
{
    clock_t start = clock();
    volatile long spin = 0;

    _TEST_TRUE(start != (clock_t) -1);
    _TEST_TRUE(CLOCKS_PER_SEC > 0);
    while (spin < SPINS) {
        spin++;
    }
    _TEST_TRUE(clock() >= start);
}

int main(void)
{
    Test_TimeClock();
    return _Test_Status();
}
