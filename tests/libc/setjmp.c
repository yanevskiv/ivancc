// (Test) Status: 0
// <setjmp.h> declares jmp_buf, an array type fit for a struct's member and an array's element (S7.13p2), setjmp, which returns 0 called directly (S7.13.1.1p3), and longjmp, which returns to the setjmp that saved a buffer (S7.13.2.1p2),
// setjmp then returning the value longjmp was given, any int but 0, for which it returns 1 (S7.13.2.1p4), in each context C99 allows a call of it (S7.13.1.1p4),
// from any depth of calls and past arguments passed on the stack, a volatile object keeping what it was last given (S7.13.2.1p3) and the caller's registers what they held,
// one buffer saved within another's reach, a buffer jumped to time after time, and longjmp a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>

#define RETURNED (-1)
#define LANDED   10
#define STRAYED  100
#define BUFS     3
#define AFTER    77
#define BASE     10
#define VALUES   6
#define DEEP     20
#define CHURNS   10
#define ROTATED  7654321

struct Holder {
    char ho_tag;
    jmp_buf ho_env;
    int ho_after;
};

static jmp_buf env;
static jmp_buf outer;
static volatile long seed = 1;
static volatile unsigned long sink;

// What six locals hold across a jump, volatile so gcc folds none of them.
static volatile long values[VALUES] = {2, 20, -299, 4096, -5000, 66667};

// Jump to a buffer unless seed is 0, so gcc keeps every frame that leads here.
static void leap(jmp_buf to, int val)
{
    if (seed != 0) {
        longjmp(to, val);
    }
}

// Jump to a buffer with a value from depth calls down.
static void jump(jmp_buf to, int depth, int val)
{
    if (depth > 0) {
        jump(to, depth - 1, val);
    }
    leap(to, val);
}

// Step a count and, the first time, jump with a value from depth calls down.
static void once(volatile int *count, jmp_buf to, int depth, int val)
{
    (*count)++;
    if (*count == 1) {
        jump(to, depth, val);
        *count = RETURNED;
    }
}

// Step a count and, up to a limit, jump with the count from depth calls down.
static void each(volatile int *count, jmp_buf to, int depth, int limit)
{
    (*count)++;
    if (*count <= limit) {
        jump(to, depth, *count);
        *count = RETURNED;
    }
}

// Jump from depth calls down, each passing a seventh argument on the stack.
static long spill(jmp_buf to, int depth, long a, long b, long c, long d, long e, long f, long g)
{
    if (depth == 0) {
        leap(to, (int) (a + b * 10 + c * 100 + d * 1000 + e * 10000 + f * 100000 + g * 1000000));
        return 0;
    }
    return spill(to, depth - 1, g, a, b, c, d, e, f) + 1;
}

// Return the sum of seven arguments, the last passed on the stack.
static long add(long a, long b, long c, long d, long e, long f, long g)
{
    return a + b + c + d + e + f + g;
}

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

// (S7.13.2.1p3) Six values a caller took before a jump should be unchanged.
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

// A jump from five calls down should land here once: return how many times.
static int inner(void)
{
    jmp_buf local;
    volatile int count = 0;

    if (setjmp(local) == 0) {
        once(&count, local, 5, 9);
    }
    return count;
}

// (S7.13p2) A jmp_buf should be an array, and take a jump in a struct too.
static void Test_SetjmpType(void)
{
    jmp_buf many[BUFS];
    volatile int count = 0;
    struct Holder holder;

    holder.ho_tag = 'h';
    holder.ho_after = AFTER;
    _TEST_TRUE(sizeof(jmp_buf) >= sizeof(env[0]));
    _TEST_UINT(sizeof(jmp_buf) % sizeof(env[0]), 0);
    _TEST_UINT(sizeof(holder.ho_env), sizeof(jmp_buf));
    _TEST_UINT(sizeof(many), BUFS * sizeof(jmp_buf));
    _TEST_TRUE(&env[0] == env);

    if (setjmp(holder.ho_env) == 0) {
        once(&count, holder.ho_env, 3, 2);
    }
    _TEST_INT(count, 1);
    _TEST_INT(holder.ho_tag, 'h');
    _TEST_INT(holder.ho_after, AFTER);

    count = 0;
    if (setjmp(many[1]) == 0) {
        once(&count, many[1], 3, 2);
    }
    _TEST_INT(count, 1);
}

// (S7.13.1.1p3) setjmp should return 0 from a direct call, whatever buffer.
static void Test_SetjmpDirect(void)
{
    jmp_buf many[BUFS];
    volatile int count = 0;
    struct Holder holder;

    if (setjmp(env) == 0) {
        count++;
    }
    _TEST_INT(count, 1);
    if (setjmp(many[2]) == 0) {
        count++;
    }
    _TEST_INT(count, 2);
    if (setjmp(holder.ho_env) == 0) {
        count++;
    }
    _TEST_INT(count, 3);
}

// (S7.13.2.1p4) setjmp should return the value longjmp was given, if not 0.
static void Test_SetjmpValues(void)
{
    volatile int count = 0;

    switch (setjmp(env)) {
    case 0:
        once(&count, env, 2, 42);
        break;
    case 42:
        count += LANDED;
        break;
    default:
        break;
    }
    _TEST_INT(count, 1 + LANDED);

    count = 0;
    if (setjmp(env) != 1) {
        once(&count, env, 2, 1);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != -1) {
        once(&count, env, 2, -1);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (-1 != setjmp(env)) {
        once(&count, env, 2, -1);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != INT_MAX) {
        once(&count, env, 2, INT_MAX);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != INT_MIN) {
        once(&count, env, 2, INT_MIN);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) < 0) {
        count += LANDED;
    } else {
        once(&count, env, 2, -7);
    }
    _TEST_INT(count, 1 + LANDED);
}

// (S7.13.2.1p4) setjmp should return 1 where longjmp was given 0.
static void Test_SetjmpZero(void)
{
    volatile int count = 0;

    if (setjmp(env) != 1) {
        once(&count, env, 2, 0);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            longjmp(env, 0);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
}

// (S7.13.1.1p4) setjmp should take a jump in each context C99 allows it.
static void Test_SetjmpContexts(void)
{
    volatile int count = 0;

    if (setjmp(env)) {
        count += LANDED;
    } else {
        once(&count, env, 1, 3);
    }
    _TEST_INT(count, 1 + LANDED);

    count = 0;
    while (! setjmp(env)) {
        once(&count, env, 1, 3);
        break;
    }
    _TEST_INT(count, 1);

    count = 0;
    for (; setjmp(env) <= 2; ) {
        each(&count, env, 1, 3);
        break;
    }
    _TEST_INT(count, 3);

    count = 0;
    do {
        count++;
        if (count == 2) {
            jump(env, 1, 8);
            count = RETURNED;
        }
        if (count != 1) {
            break;
        }
    } while (setjmp(env) == 0);
    _TEST_INT(count, 2);

    count = 0;
    if (2 > setjmp(env)) {
        once(&count, env, 1, 2);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) >= 5) {
        count += LANDED;
    } else {
        once(&count, env, 1, 5);
    }
    _TEST_INT(count, 1 + LANDED);

    count = 0;
    if (0 == setjmp(env)) {
        once(&count, env, 1, 4);
    }
    _TEST_INT(count, 1);

    count = 0;
    setjmp(env);
    once(&count, env, 1, 6);
    _TEST_INT(count, 2);

    count = 0;
    (void) setjmp(env);
    once(&count, env, 1, 6);
    _TEST_INT(count, 2);
}

// (S7.13.2.1p2) longjmp should return from many calls down, past the stack.
static void Test_SetjmpDepth(void)
{
    volatile int count = 0;

    if (setjmp(env) == 0) {
        once(&count, env, DEEP, 7);
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) != ROTATED) {
        count++;
        if (count == 1) {
            spill(env, DEEP + 1, 1, 2, 3, 4, 5, 6, 7);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
    _TEST_INT(add(1, 2, 3, 4, 5, 6, 7), 28);
}

// (S7.13.2.1p3) A jump should leave volatile objects and unchanged locals.
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

// (S7.13.2.1p2) A jump should land at its own buffer's setjmp, among others.
static void Test_SetjmpNested(void)
{
    jmp_buf local;
    volatile int step = 0;
    volatile int count = 0;
    volatile int nested = 0;

    if (setjmp(outer) == 0) {
        step = step * BASE + 1;
        if (setjmp(local) == 0) {
            step = step * BASE + 2;
            once(&nested, local, 4, 1);
        }
        step = step * BASE + 3;
        _TEST_INT(inner(), 1);
        step = step * BASE + 4;
        once(&count, outer, 4, 1);
    }
    _TEST_INT(step, 1234);
    _TEST_INT(nested, 1);
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(outer) == 0) {
        if (setjmp(env) == 0) {
            once(&count, outer, 2, 1);
        }
        count += STRAYED;
    }
    _TEST_INT(count, 1);
}

// (S7.13.2.1p2) A buffer should take jump after jump, each with its value.
static void Test_SetjmpAgain(void)
{
    volatile int count = 0;

    while (setjmp(env) < 5) {
        each(&count, env, 3, 5);
        break;
    }
    _TEST_INT(count, 5);
}

// (S7.1.4p1) A pointer and a call in parentheses should reach longjmp.
static void Test_SetjmpCalls(void)
{
    volatile int count = 0;
    void (*jumper)(jmp_buf, int) = longjmp;

    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            jumper(env, 3);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);

    count = 0;
    if (setjmp(env) == 0) {
        count++;
        if (count == 1) {
            (longjmp)(env, 3);
            count = RETURNED;
        }
    }
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_SetjmpType();
    Test_SetjmpDirect();
    Test_SetjmpValues();
    Test_SetjmpZero();
    Test_SetjmpContexts();
    Test_SetjmpDepth();
    Test_SetjmpLocals();
    Test_SetjmpNested();
    Test_SetjmpAgain();
    Test_SetjmpCalls();
    return _Test_Status();
}
