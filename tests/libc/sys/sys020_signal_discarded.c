// (Test) Status: 0
// Ignoring a blocked signal discards its pending instance, so a handler installed after never sees it, on the machine and in the emulator.

#define SYS_RT_SIGACTION   13
#define SYS_RT_SIGPROCMASK 14
#define SYS_GETPID         39
#define SYS_KILL           62

#define SIG_BLOCK   0
#define SIG_UNBLOCK 1

#define SIG_IGN 1

#define SA_RESTORER 0x04000000

#define SIGTERM 15

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall");
#endif

static volatile int got;

static void on_signal(int sig)
{
    got = sig;
}

// Install a handler for sig with flags, blocking mask while it runs.
static long install(int sig, void (*handler)(int), unsigned long flags, unsigned long mask)
{
    struct sigaction_linux act = {0};

    act.handler = handler;
    act.flags = flags | SA_RESTORER;
    act.restorer = restorer;
    act.mask = mask;
    return sys(SYS_RT_SIGACTION, sig, (long) &act, 0, 8);
}

// Send the program a signal.
static long kill_self(int sig)
{
    return sys(SYS_KILL, sys(SYS_GETPID, 0, 0, 0, 0), sig, 0, 0);
}

// Change the signal mask by how with set, and return the old one.
static unsigned long mask(int how, unsigned long set)
{
    unsigned long old = 0;

    sys(SYS_RT_SIGPROCMASK, how, set ? (long) &set : 0, (long) &old, 8);
    return old;
}

// The bit a signal takes in a mask.
static unsigned long bit(int sig)
{
    return 1UL << (sig - 1);
}

int main(void)
{
    if (install(SIGTERM, on_signal, 0, 0) != 0) {
        return 1;
    }
    mask(SIG_BLOCK, bit(SIGTERM));
    kill_self(SIGTERM);
    if (install(SIGTERM, (void (*)(int)) SIG_IGN, 0, 0) != 0 || install(SIGTERM, on_signal, 0, 0) != 0) {
        return 2;
    }
    mask(SIG_UNBLOCK, bit(SIGTERM));
    return got;
}
