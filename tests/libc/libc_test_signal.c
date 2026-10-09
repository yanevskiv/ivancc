// (Test) Status: 0
// <signal.h> defines sig_atomic_t, the handlers and the signals, signal, which installs a handler, and raise, which runs it (S7.14).

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

static volatile sig_atomic_t got;
static volatile sig_atomic_t count;
static jmp_buf env;

static void on_signal(int sig)
{
    got = sig;
    count++;
}

static void on_other(int sig)
{
    got = -sig;
}

static void on_reinstall(int sig)
{
    signal(sig, on_reinstall);
    count++;
}

static void on_leave(int sig)
{
    longjmp(env, sig);
}

static bool raises(int sig)
{
    got = 0;
    count = 0;
    if (signal(sig, on_signal) == SIG_ERR) return false;
    if (raise(sig) != 0) return false;
    return got == sig and count == 1;
}

int main(void)
{
    static const int sigs[6] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};
    void (*(*set)(int, void (*)(int)))(int) = signal;
    int (*send)(int) = raise;
    void (*first)(int);
    volatile sig_atomic_t atom = SIG_ATOMIC_MAX;
    int jumped;

    for (int i = 0; i < 6; i++) {
        if (sigs[i] <= 0) return 1;
        for (int j = 0; j < i; j++) {
            if (sigs[i] == sigs[j]) return 1;
        }
    }

    if (SIG_DFL == SIG_ERR or SIG_DFL == SIG_IGN or SIG_ERR == SIG_IGN) return 2;
    if (SIG_DFL == on_signal or SIG_ERR == on_signal or SIG_IGN == on_signal) return 2;

    if (atom != SIG_ATOMIC_MAX) return 3;
    atom = SIG_ATOMIC_MIN;
    if (atom != SIG_ATOMIC_MIN) return 3;

    first = signal(SIGTERM, on_signal);
    if (first != SIG_DFL and first != SIG_IGN) return 4;
    if (signal(SIGTERM, on_other) != on_signal) return 4;
    if (signal(SIGTERM, SIG_DFL) != on_other) return 4;
    if (signal(SIGTERM, first) != SIG_DFL) return 4;

    for (int i = 0; i < 6; i++) {
        if (not raises(sigs[i])) return 5;
    }

    got = 0;
    if (signal(SIGTERM, on_other) == SIG_ERR or raise(SIGTERM) != 0 or got != -SIGTERM) return 6;

    if (signal(SIGTERM, SIG_IGN) == SIG_ERR) return 7;
    if (raise(SIGTERM) != 0) return 7;
    if (signal(SIGTERM, SIG_DFL) != SIG_IGN) return 7;

    count = 0;
    if (signal(SIGFPE, on_reinstall) == SIG_ERR) return 8;
    if (raise(SIGFPE) != 0 or raise(SIGFPE) != 0 or count != 2) return 8;

    errno = 0;
    if (signal(-1, on_signal) == SIG_ERR and errno <= 0) return 9;
    errno = 0;
    if (signal(INT_MAX, on_signal) == SIG_ERR and errno <= 0) return 9;

    got = 0;
    if ((*set)(SIGILL, on_signal) == SIG_ERR or (send)(SIGILL) != 0 or got != SIGILL) return 10;
    first = (signal)(SIGILL, on_other);
    if (first != on_signal and first != SIG_DFL) return 10;
    got = 0;
    if ((raise)(SIGILL) != 0 or got != -SIGILL) return 10;

    if (signal(SIGINT, on_leave) == SIG_ERR) return 11;
    jumped = setjmp(env);
    if (jumped == 0) {
        raise(SIGINT);
        return 11;
    }
    if (jumped != SIGINT) return 11;

    return 0;
}
