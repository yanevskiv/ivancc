// (Test) Status: 139
// A handler installed without SA_RESTORER has nothing to return through, so its signal ends the program by SIGSEGV, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_GETPID       39
#define SYS_EXIT         60
#define SYS_KILL         62

#define SIGUSR1 10

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret");
#endif

static void on_signal(int sig)
{
    sys(SYS_EXIT, sig, 0, 0, 0);
}

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = on_signal;
    if (sys(SYS_RT_SIGACTION, SIGUSR1, (long) &act, 0, 8) != 0) {
        return 1;
    }
    sys(SYS_KILL, sys(SYS_GETPID, 0, 0, 0, 0), SIGUSR1, 0, 0);
    return 2;
}
