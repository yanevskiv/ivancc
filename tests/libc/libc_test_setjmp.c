// (Test) Status: 0
// <setjmp.h> declares jmp_buf, setjmp and longjmp, which jumps back to the setjmp that saved a buffer (S7.13).

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

struct holder {
    char tag;
    jmp_buf env;
    int after;
};

static jmp_buf env;
static jmp_buf outer;
static volatile long seed = 1;
static volatile unsigned long sink;
static volatile long values[6] = {2, 20, -299, 4096, -5000, 66667};

// Jump to `to` unless seed is 0, so gcc keeps every frame that leads here.
static void leap(jmp_buf to, int val)
{
    if (seed != 0) {
        longjmp(to, val);
    }
}

static void jump(jmp_buf to, int depth, int val)
{
    if (depth > 0) {
        jump(to, depth - 1, val);
    }
    leap(to, val);
}

static long spill(jmp_buf to, int depth, long a, long b, long c, long d, long e, long f, long g)
{
    if (depth == 0) {
        leap(to, (int) (a + b * 10 + c * 100 + d * 1000 + e * 10000 + f * 100000 + g * 1000000));
        return 0;
    }
    return spill(to, depth - 1, g, a, b, c, d, e, f) + 1;
}

static long add(long a, long b, long c, long d, long e, long f, long g)
{
    return a + b + c + d + e + f + g;
}

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

static int churned(void)
{
    jmp_buf local;

    if (setjmp(local) == 0) {
        sink = churn(local, 10, seed);
    }
    return 1;
}

static bool unchanged(long k1, long k2, long k3, long k4, long k5, long k6)
{
    return k1 == 2 and k2 == 20 and k3 == -299 and k4 == 4096 and k5 == -5000 and k6 == 66667;
}

static int (*volatile around)(void) = churned;
static bool (*volatile check)(long, long, long, long, long, long) = unchanged;

// Hold six values in gcc's callee-saved registers across a jump in a callee.
static bool keep(void)
{
    long k1 = values[0];
    long k2 = values[1];
    long k3 = values[2];
    long k4 = values[3];
    long k5 = values[4];
    long k6 = values[5];

    if (around() != 1) return false;
    return check(k1, k2, k3, k4, k5, k6);
}

static bool (*volatile kept)(void) = keep;

static int inner(void)
{
    jmp_buf local;
    volatile int count = 0;

    if (setjmp(local) == 0) {
        count++;
        jump(local, 5, 9);
    }
    return count;
}

static bool is_array(size_t whole, size_t element)
{
    return whole >= element and whole % element == 0;
}

int main(void)
{
    struct holder holder;
    jmp_buf many[3];
    void (*jumper)(jmp_buf, int) = longjmp;
    volatile int count;
    volatile int step;
    volatile long vl;
    volatile char vc;
    long k1 = seed + 1;
    long k2 = seed * 20;
    long k3 = seed - 300;
    long k4 = seed << 12;
    long k5 = seed * -5000;
    long k6 = seed + 66666;

    holder.tag = 'h';
    holder.after = 77;
    if (not is_array(sizeof(jmp_buf), sizeof(env[0])) or sizeof(holder.env) != sizeof(jmp_buf)) return 1;
    if (sizeof(many) != 3 * sizeof(jmp_buf) or &env[0] != env) return 1;
    count = 0;
    if (setjmp(holder.env) == 0) {
        count++;
        jump(holder.env, 3, 2);
    }
    if (count != 1 or holder.tag != 'h' or holder.after != 77) return 1;
    count = 0;
    if (setjmp(many[1]) == 0) {
        count++;
        jump(many[1], 3, 2);
    }
    if (count != 1) return 1;

    if (setjmp(env) != 0) return 2;
    if (setjmp(many[2]) != 0) return 2;
    if (setjmp(holder.env) != 0) return 2;

    count = 0;
    switch (setjmp(env)) {
    case 0:
        count++;
        jump(env, 2, 42);
        break;
    case 42:
        break;
    default:
        return 3;
    }
    if (count != 1) return 3;
    count = 0;
    if (setjmp(env) != 1) {
        if (count++ > 0) return 3;
        jump(env, 2, 1);
    }
    count = 0;
    if (setjmp(env) != -1) {
        if (count++ > 0) return 3;
        jump(env, 2, -1);
    }
    count = 0;
    if (-1 != setjmp(env)) {
        if (count++ > 0) return 3;
        jump(env, 2, -1);
    }
    count = 0;
    if (setjmp(env) != INT_MAX) {
        if (count++ > 0) return 3;
        jump(env, 2, INT_MAX);
    }
    count = 0;
    if (setjmp(env) != INT_MIN) {
        if (count++ > 0) return 3;
        jump(env, 2, INT_MIN);
    }
    count = 0;
    if (setjmp(env) < 0) {
        if (count != 1) return 3;
    } else {
        if (count++ > 0) return 3;
        jump(env, 2, -7);
    }

    count = 0;
    if (setjmp(env) != 1) {
        if (count++ > 0) return 4;
        jump(env, 2, 0);
    }
    count = 0;
    if (setjmp(env) == 0) {
        if (count++ > 0) return 4;
        longjmp(env, 0);
    }
    if (count != 1) return 4;

    count = 0;
    if (setjmp(env)) {
        count += 10;
    } else {
        count++;
        jump(env, 1, 3);
    }
    if (count != 11) return 5;
    count = 0;
    while (not setjmp(env)) {
        count++;
        jump(env, 1, 3);
    }
    if (count != 1) return 5;
    count = 0;
    for (; setjmp(env) <= 2; ) {
        count++;
        jump(env, 1, count);
    }
    if (count != 3) return 5;
    count = 0;
    do {
        count++;
        if (count > 4) return 5;
        if (count == 2) jump(env, 1, 8);
    } while (setjmp(env) == 0);
    if (count != 2) return 5;
    count = 0;
    if (2 > setjmp(env)) {
        count++;
        jump(env, 1, 2);
    }
    if (count != 1) return 5;
    count = 0;
    if (setjmp(env) >= 5) {
        count += 10;
    } else {
        count++;
        jump(env, 1, 5);
    }
    if (count != 11) return 5;
    count = 0;
    if (0 == setjmp(env)) {
        count++;
        jump(env, 1, 4);
    }
    if (count != 1) return 5;
    count = 0;
    setjmp(env);
    if (count++ == 0) jump(env, 1, 6);
    if (count != 2) return 5;
    count = 0;
    (void) setjmp(env);
    if (count++ == 0) jump(env, 1, 6);
    if (count != 2) return 5;

    count = 0;
    if (setjmp(env) == 0) {
        count++;
        jump(env, 20, 7);
    }
    if (count != 1) return 6;
    count = 0;
    if (setjmp(env) != 7654321) {
        if (count++ > 0) return 6;
        spill(env, 21, 1, 2, 3, 4, 5, 6, 7);
    }
    if (count != 1 or add(1, 2, 3, 4, 5, 6, 7) != 28) return 6;

    count = 0;
    vl = 5;
    vc = 'a';
    if (setjmp(env) == 0) {
        count++;
        vl = -123456789012L;
        vc = 'z';
        churn(env, 10, seed);
    }
    if (count != 1 or vl != -123456789012L or vc != 'z') return 7;
    if (k1 != 2 or k2 != 20 or k3 != -299 or k4 != 4096 or k5 != -5000 or k6 != 66667) return 7;
    if (not kept()) return 7;

    step = 0;
    if (setjmp(outer) == 0) {
        step = step * 10 + 1;
        if (setjmp(many[0]) == 0) {
            step = step * 10 + 2;
            jump(many[0], 4, 1);
        }
        step = step * 10 + 3;
        if (inner() != 1) return 8;
        step = step * 10 + 4;
        jump(outer, 4, 1);
    }
    if (step != 1234) return 8;
    count = 0;
    if (setjmp(outer) == 0) {
        if (setjmp(env) == 0) {
            count++;
            jump(outer, 2, 1);
        }
        return 8;
    }
    if (count != 1) return 8;

    count = 0;
    while (setjmp(env) < 5) {
        count++;
        jump(env, 3, count);
    }
    if (count != 5) return 9;

    count = 0;
    if (setjmp(env) == 0) {
        count++;
        jumper(env, 3);
    }
    if (count != 1) return 10;
    count = 0;
    if (setjmp(env) == 0) {
        count++;
        (longjmp)(env, 3);
    }
    if (count != 1) return 10;
    return 0;
}
