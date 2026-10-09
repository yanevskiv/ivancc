// (Test) Status: 138
// SA_RESETHAND runs a handler once and restores the default, so the second SIGUSR1 ends the program, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_GETPID       39
#define SYS_KILL         62

#define SA_RESTORER  0x04000000
#define SA_RESETHAND 0x80000000

#define SIGUSR1 10

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

int main(void)
{
    if (install(SIGUSR1, on_signal, SA_RESETHAND, 0) != 0) {
        return 1;
    }
    kill_self(SIGUSR1);
    if (got != SIGUSR1) {
        return 2;
    }
    kill_self(SIGUSR1);
    return 3;
}
