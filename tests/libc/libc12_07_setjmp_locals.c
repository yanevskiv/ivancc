// (Test) Status: 0
// A test file for <setjmp.h> objects across a jump.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

#define VALUES 6
#define CHURNS 10

static volatile unsigned long sink;

// What six locals hold across a jump, volatile so gcc folds none of them.
static volatile long values[VALUES] = {2, 20, -299, 4096, -5000, 66667};

// Jump from depth calls down, each holding six values across its own call.
static unsigned long churn(jmp_buf to, int depth, unsigned long value)
{
    unsigned long a = value + 1 + seed;
    unsigned long b = value * 3 + seed;
    unsigned long c = (value ^ 5) + seed;
    unsigned long d = value - 7 + seed;
    unsigned long e = value * value + seed;
    unsigned long f = value + 11 + seed;

    if (depth == 0) {
        leap(to, 1);
        return value;
    }
    return ((((churn(to, depth - 1, a + b) * a + b) * c + d) * e + f) ^ a) * b;
}

// A jump out of churn's calls should land here: return 1 once it has.
static int churned(void)
{
    jmp_buf local;

    if (setjmp(local) == 0) {
        sink = churn(local, CHURNS, seed);
    }
    return 1;
}

// [C99 S7.13.2.1p3] Six values a caller took before a jump should be unchanged.
static bool unchanged(long k1, long k2, long k3, long k4, long k5, long k6)
{
    int lost = 0;

    lost += ! _TEST_INT(k1, values[0]);
    lost += ! _TEST_INT(k2, values[1]);
    lost += ! _TEST_INT(k3, values[2]);
    lost += ! _TEST_INT(k4, values[3]);
    lost += ! _TEST_INT(k5, values[4]);
    lost += ! _TEST_INT(k6, values[5]);
    return lost == 0;
}

// Called through volatile pointers, so gcc inlines neither into keep.
static int (*volatile around)(void) = churned;
static bool (*volatile check)(long, long, long, long, long, long) = unchanged;

// Hold six values in gcc's callee-saved registers across a jump in a callee.
static bool keep(void)
{
    int lost = 0;
    long k1 = values[0];
    long k2 = values[1];
    long k3 = values[2];
    long k4 = values[3];
    long k5 = values[4];
    long k6 = values[5];

    lost += ! _TEST_INT(around(), 1);
    lost += ! check(k1, k2, k3, k4, k5, k6);
    return lost == 0;
}

// Called through a volatile pointer, so keep has registers of its own.
static bool (*volatile kept)(void) = keep;

// [C99 S7.13.2.1p3] A jump should leave volatile objects and unchanged locals.
static void Test_SetjmpLocals(void)
{
    long k1 = seed + 1;
    long k2 = seed * 20;
    long k3 = seed - 300;
    long k4 = seed << 12;
    long k5 = seed * -5000;
    long k6 = seed + 66666;
    volatile long vl = 5;
    volatile char vc = 'a';
    volatile int count = 0;

    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            vl = -123456789012L;
            vc = 'z';
            churn(env, CHURNS, seed);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
    _TEST_INT(vl, -123456789012L);
    _TEST_INT(vc, 'z');
    _TEST_TRUE(unchanged(k1, k2, k3, k4, k5, k6));
    _TEST_TRUE(kept());
}

int main(void)
{
    Test_SetjmpLocals();
    return _Test_Status();
}
