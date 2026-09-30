// (Test) Status: 200
// bug034. The constant folder divided signed operands with the host's own
// division, and x86 traps on the one quotient that does not fit, so
// `LONG_MIN / -1` and `LONG_MIN % -1` killed the compiler with SIGFPE. Like
// gcc, the division wraps to LONG_MIN and the remainder is 0.

#define LONG_MIN (-9223372036854775807L - 1)

enum {
    QUOTIENT_WRAPS = LONG_MIN / -1 == LONG_MIN,
    REMAINDER_ZERO = LONG_MIN % -1 == 0
};

long quotient = LONG_MIN / -1;
long remainder = LONG_MIN % -1;

int main()
{
    if (QUOTIENT_WRAPS != 1 || REMAINDER_ZERO != 1) return 1;
    if (quotient != LONG_MIN || remainder != 0) return 2;

    switch (LONG_MIN) {
        case LONG_MIN / -1: return 200;
    }
    return 3;
}
