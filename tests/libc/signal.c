// (Test) Status: 0
// <signal.h> defines sig_atomic_t, which holds its limits, SIG_DFL, SIG_ERR and SIG_IGN, distinct and no function's address, and SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV and SIGTERM, distinct positive int constants (S7.14p2-p3),
// signal, which installs a handler and returns the one before it, and gives SIG_ERR and a positive errno for a signal it cannot handle (S7.14.1.1), and raise, which runs the handler with the signal's number and returns 0 (S7.14.2.1),
// a handler installing itself again, an ignored signal doing nothing, and a handler leaving by longjmp, each a function a pointer reaches, and one a call reaches past any macro of its name (S7.1.4p1).

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
#include <assert.h>
#include <time.h>
#include <signal.h>

#define NONE    (-1)
#define SIGNALS 6
#define TWICE   2

// The signals C99 names.
static const int sigs[SIGNALS] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};

static volatile sig_atomic_t got;
static volatile sig_atomic_t count;
static jmp_buf env;

// Note the signal, and that a handler ran.
static void on_signal(int sig)
{
    got = sig;
    count++;
}

// Note the signal negated, to tell this handler from the other.
static void on_other(int sig)
{
    got = -sig;
}

// Install this handler again, as one that runs twice must where it is reset.
static void on_reinstall(int sig)
{
    signal(sig, on_reinstall);
    count++;
}

// Note the signal and leave without returning to raise.
static void on_leave(int sig)
{
    got = sig;
    longjmp(env, 1);
}

// Return the place of the first signal no greater than 0, or NONE.
static int nonpositive(void)
{
    int i;

    for (i = 0; i < SIGNALS; i++) {
        if (sigs[i] <= 0) {
            return i;
        }
    }
    return NONE;
}

// Return the place of the first signal equal to one before it, or NONE.
static int repeated(void)
{
    int i;
    int j;

    for (i = 0; i < SIGNALS; i++) {
        for (j = 0; j < i; j++) {
            if (sigs[i] == sigs[j]) {
                return i;
            }
        }
    }
    return NONE;
}

// Tell whether raise runs a signal's handler once, with the signal's number.
static bool raises(int sig)
{
    int lost = 0;

    got = 0;
    count = 0;
    if (! _TEST_TRUE(signal(sig, on_signal) != SIG_ERR)) {
        return false;
    }
    lost += ! _TEST_INT(raise(sig), 0);
    lost += ! _TEST_INT(got, sig);
    lost += ! _TEST_INT(count, 1);
    return lost == 0;
}

// Tell whether signal refuses a number, and says why in errno.
static bool refuses(int sig)
{
    int lost = 0;
    int number;
    void (*handler)(int);

    errno = 0;
    handler = signal(sig, on_signal);
    number = errno;
    lost += ! _TEST_TRUE(handler == SIG_ERR);
    lost += ! _TEST_TRUE(number > 0);
    return lost == 0;
}

// The signals are distinct and positive, the three handlers no function's.
static void Test_SignalMacros(void)
{
    _TEST_INT(nonpositive(), NONE);
    _TEST_INT(repeated(), NONE);
    _TEST_UINT(sizeof(SIGABRT), sizeof(int));
    _TEST_TRUE(SIG_DFL != SIG_ERR);
    _TEST_TRUE(SIG_DFL != SIG_IGN);
    _TEST_TRUE(SIG_ERR != SIG_IGN);
    _TEST_TRUE(SIG_DFL != on_signal);
    _TEST_TRUE(SIG_ERR != on_signal);
    _TEST_TRUE(SIG_IGN != on_signal);
}

// A sig_atomic_t holds each of its limits.
static void Test_SignalAtomic(void)
{
    volatile sig_atomic_t atom = SIG_ATOMIC_MAX;

    _TEST_INT(atom, SIG_ATOMIC_MAX);
    atom = SIG_ATOMIC_MIN;
    _TEST_INT(atom, SIG_ATOMIC_MIN);
    atom = 0;
    _TEST_INT(atom, 0);
}

// signal returns the handler it replaces, the default or none at startup.
static void Test_SignalInstalls(void)
{
    void (*first)(int) = signal(SIGTERM, on_signal);

    _TEST_TRUE(first == SIG_DFL || first == SIG_IGN);
    _TEST_TRUE(signal(SIGTERM, on_other) == on_signal);
    _TEST_TRUE(signal(SIGTERM, SIG_DFL) == on_other);
    _TEST_TRUE(signal(SIGTERM, first) == SIG_DFL);
    _TEST_TRUE(signal(SIGTERM, first) == first);
}

// raise runs the handler of each signal C99 names.
static void Test_SignalRaises(void)
{
    _TEST_TRUE(raises(SIGABRT));
    _TEST_TRUE(raises(SIGFPE));
    _TEST_TRUE(raises(SIGILL));
    _TEST_TRUE(raises(SIGINT));
    _TEST_TRUE(raises(SIGSEGV));
    _TEST_TRUE(raises(SIGTERM));
}

// raise runs the handler last installed, none for a signal ignored.
static void Test_SignalHandlers(void)
{
    got = 0;
    if (_TEST_TRUE(signal(SIGTERM, on_other) != SIG_ERR)) {
        _TEST_INT(raise(SIGTERM), 0);
        _TEST_INT(got, -SIGTERM);
    }

    got = 0;
    if (_TEST_TRUE(signal(SIGTERM, SIG_IGN) != SIG_ERR)) {
        _TEST_INT(raise(SIGTERM), 0);
        _TEST_INT(got, 0);
    }
    _TEST_TRUE(signal(SIGTERM, SIG_DFL) == SIG_IGN);

    count = 0;
    if (_TEST_TRUE(signal(SIGFPE, on_reinstall) != SIG_ERR)) {
        _TEST_INT(raise(SIGFPE), 0);
        _TEST_INT(raise(SIGFPE), 0);
        _TEST_INT(count, TWICE);
    }
}

// signal refuses a number that is no signal's.
static void Test_SignalRefused(void)
{
    _TEST_TRUE(refuses(-1));
    _TEST_TRUE(refuses(0));
    _TEST_TRUE(refuses(INT_MAX));
}

// A handler that leaves by longjmp never returns to raise.
static void Test_SignalLeaves(void)
{
    volatile int returned = 0;

    got = 0;
    if (_TEST_TRUE(signal(SIGINT, on_leave) != SIG_ERR)) {
        if (setjmp(env) == 0) {
            raise(SIGINT);
            returned = 1;
        }
        _TEST_INT(got, SIGINT);
        _TEST_INT(returned, 0);
    }
}

// A pointer reaches each function, and a call in parentheses any macro's.
static void Test_SignalCalls(void)
{
    int (*send)(int) = raise;
    void (*before)(int);
    void (*(*set)(int, void (*)(int)))(int) = signal;

    got = 0;
    if (_TEST_TRUE(set(SIGILL, on_signal) != SIG_ERR)) {
        _TEST_INT(send(SIGILL), 0);
        _TEST_INT(got, SIGILL);
    }
    before = (signal)(SIGILL, on_other);
    if (_TEST_TRUE(before == on_signal || before == SIG_DFL)) {
        _TEST_INT((raise)(SIGILL), 0);
        _TEST_INT(got, -SIGILL);
    }
}

int main(void)
{
    Test_SignalMacros();
    Test_SignalAtomic();
    Test_SignalInstalls();
    Test_SignalRaises();
    Test_SignalHandlers();
    Test_SignalRefused();
    Test_SignalLeaves();
    Test_SignalCalls();
    return _Test_Status();
}
